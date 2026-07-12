#include "IOManagement.h"

volatile Digital_Data digital_data;
volatile float regen_brake;
volatile uint8_t regen_brake_percent = 0;
volatile float throttle;
volatile uint16_t number_reads = 0;
volatile bool hazards = 0;
volatile uint8_t drive_mode = 0;
portMUX_TYPE stateMux = portMUX_INITIALIZER_UNLOCKED;

static bool headlight_state = false;
static bool last_headlight_input = false;
static bool left_blink_state = false;
static bool last_left_blink_input = false;
static bool right_blink_state = false;
static bool last_right_blink_input = false;
static bool hazards_state = false;
static bool last_hazards_input = false;
static bool direction_switch_state = true;
static bool last_direction_switch_input = false;
static bool direction_input_armed = false;
static bool drive_mode_state = false;
static bool last_drive_mode_input = false;
static bool last_regen_button_input = false;

static const uint16_t MAX_ANALOG_VALUE = THROTTLE_SENT_MAX;

static bool toggleOnPress(bool input, bool &state, bool &last_input) {
    if (input && !last_input) {
        state = !state;
    }

    last_input = input;
    return state;
}

void sampleIO() {
    // 1. Read all pins into local variables first (outside critical section to minimize interrupt latency)
    bool local_regen_button = digitalRead(REGEN_BRAKE_PIN);
    uint16_t local_throttle_raw = analogRead(THROTTLE_PIN);
    bool local_headlight = digitalRead(HEADLIGHT_PIN);
    bool local_left_blink = digitalRead(LEFT_BLINK_PIN);
    bool local_right_blink = digitalRead(RIGHT_BLINK_PIN);
    bool local_direction_switch = digitalRead(DIRECTION_SWITCH_PIN);
    bool local_horn = digitalRead(HORN_PIN);
    bool local_hazards = digitalRead(HAZARDS_PIN);
    bool local_drive_mode = digitalRead(DRIVE_MODE_PIN);

    // 2. Perform calibration calculations
    float pedal_calibrated = calibratePedal(local_throttle_raw);

    float local_regen_brake = 0.0f;
    uint8_t local_regen_brake_percent = 0;
    float local_throttle = 0.0f;

    if (local_regen_button) {
        // While the regen button is pressed, the acceleration pedal acts as a regen brake pedal.
        local_regen_brake = pedal_calibrated;
        local_regen_brake_percent = (uint8_t)((pedal_calibrated * 100.0f) / (float)THROTTLE_SENT_MAX + 0.5f);
        local_throttle = 0.0f;
    } else {
        // When released, regen brake is set back to 0.
        local_regen_brake = 0.0f;
        local_regen_brake_percent = 0;
        local_throttle = pedal_calibrated;
    }

    // Print local throttle reading (raw ADC and voltage)
#ifdef DEBUG_PRINTS
    {
        uint16_t throttle_raw_print = (uint16_t)local_throttle;
        float throttle_voltage = 3.3f * (float)throttle_raw_print / 4095.0f;
        Serial.printf("Local throttle: raw=%u voltage=%.3fV\n", throttle_raw_print, throttle_voltage);
    }
#endif

    // 3. Write updates to shared volatile state under the spinlock critical section
    portENTER_CRITICAL(&stateMux);

    last_regen_button_input = local_regen_button;
    regen_brake = local_regen_brake;
    regen_brake_percent = local_regen_brake_percent;
    throttle = local_throttle;

    digital_data.headlight = toggleOnPress(local_headlight, headlight_state, last_headlight_input);
    if (!direction_input_armed) {
        // Ignore the first edge after boot so switch bounce cannot flip us to reverse.
        last_direction_switch_input = local_direction_switch;
        digital_data.direction_switch = direction_switch_state;
        direction_input_armed = true;
    } else {
        digital_data.direction_switch = toggleOnPress(
            local_direction_switch, direction_switch_state, last_direction_switch_input);
    }
    digital_data.horn = local_horn;

    bool was_hazards = hazards_state;
    hazards = toggleOnPress(local_hazards, hazards_state, last_hazards_input);
    if (hazards && !was_hazards) {
        left_blink_state = false;
        right_blink_state = false;
        digital_data.left_blink = false;
        digital_data.right_blink = false;
    }

    if (!hazards) {
        digital_data.left_blink = toggleOnPress(local_left_blink, left_blink_state, last_left_blink_input);
        digital_data.right_blink = toggleOnPress(local_right_blink, right_blink_state, last_right_blink_input);
    } else {
        last_left_blink_input = local_left_blink;
        last_right_blink_input = local_right_blink;
    }
    drive_mode = toggleOnPress(local_drive_mode, drive_mode_state, last_drive_mode_input);

    number_reads++;

    portEXIT_CRITICAL(&stateMux);
}

void initIO() {
    // Initialize digital pins
    pinMode(REGEN_BRAKE_PIN, INPUT);
    pinMode(HEADLIGHT_PIN, INPUT);
    pinMode(LEFT_BLINK_PIN, INPUT);
    pinMode(RIGHT_BLINK_PIN, INPUT);
    pinMode(DIRECTION_SWITCH_PIN, INPUT);
    pinMode(HORN_PIN, INPUT);
    pinMode(HAZARDS_PIN, INPUT);
    pinMode(DRIVE_MODE_PIN, INPUT);

    last_headlight_input = digitalRead(HEADLIGHT_PIN);
    last_left_blink_input = digitalRead(LEFT_BLINK_PIN);
    last_right_blink_input = digitalRead(RIGHT_BLINK_PIN);
    last_direction_switch_input = digitalRead(DIRECTION_SWITCH_PIN);
    last_hazards_input = digitalRead(HAZARDS_PIN);
    last_drive_mode_input = digitalRead(DRIVE_MODE_PIN);
    last_regen_button_input = digitalRead(REGEN_BRAKE_PIN);

    // Default to forward on boot; first sampleIO() only seeds edge detection.
    direction_switch_state = true;
    direction_input_armed = false;
    portENTER_CRITICAL(&stateMux);
    digital_data.direction_switch = true;
    portEXIT_CRITICAL(&stateMux);

    // Seed inputs once at startup so values are valid before first task execution.
    sampleIO();
}
