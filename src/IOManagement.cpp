// IOManagement: sample steering-wheel pins into shared state
// Buttons use rising-edge toggle - Pedal is calibrated to 0..THROTTLE_SENT_MAX
// Writers take stateMux - Readers (CAN TX, display) take the same mux
#include "IOManagement.h"

volatile Digital_Data digital_data;
volatile uint8_t regen_brake_percent = 0;
volatile float throttle;
volatile bool hazards = 0;
volatile uint8_t drive_mode = 0;
volatile uint16_t lap_count = 0;
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
static bool last_crz_set_input = false;
static bool last_crz_reset_input = false;

static bool toggleOnPress(bool input, bool &state, bool &last_input) {
    // Flip state on rising edge, hold last_input for the next sample
    if (input && !last_input) {
        state = !state;
    }

    last_input = input;
    return state;
}

void sampleIO() {
    // Read pins first outside the critical section to keep the lock short
    bool local_regen_button = digitalRead(REGEN_BRAKE_PIN);
    uint16_t local_throttle_raw = analogRead(THROTTLE_PIN);
    bool local_headlight = digitalRead(HEADLIGHT_PIN);
    bool local_left_blink = digitalRead(LEFT_BLINK_PIN);
    bool local_right_blink = digitalRead(RIGHT_BLINK_PIN);
    bool local_direction_switch = digitalRead(DIRECTION_SWITCH_PIN);
    bool local_horn = digitalRead(HORN_PIN);
    bool local_hazards = digitalRead(HAZARDS_PIN);
    bool local_drive_mode = digitalRead(DRIVE_MODE_PIN);
    bool local_crz_set = digitalRead(CRZ_SET_PIN);
    bool local_crz_reset = digitalRead(CRZ_RESET_PIN);

    float pedal_calibrated = calibratePedal(local_throttle_raw);

    uint8_t local_regen_brake_percent = 0;
    float local_throttle = 0.0f;

    if (local_regen_button) {
        // While the regen button is pressed, the accel pedal sets regen, throttle held 0
        local_regen_brake_percent = (uint8_t)((pedal_calibrated * 100.0f) / (float)THROTTLE_SENT_MAX + 0.5f);
        local_throttle = 0.0f;
    } else {
        local_regen_brake_percent = 0;
        local_throttle = pedal_calibrated;
    }

#ifdef DEBUG_PRINTS
    {
        uint16_t throttle_raw_print = (uint16_t)local_throttle;
        float throttle_voltage = 3.3f * (float)throttle_raw_print / 4095.0f;
        Serial.printf("Local throttle: raw=%u voltage=%.3fV\n", throttle_raw_print, throttle_voltage);
    }
#endif

    portENTER_CRITICAL(&stateMux);

    regen_brake_percent = local_regen_brake_percent;
    throttle = local_throttle;

    digital_data.headlight = toggleOnPress(local_headlight, headlight_state, last_headlight_input);
    if (!direction_input_armed) {
        // Ignore the first edge after boot so switch bounce cannot flip us to reverse
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
        // Hazards on clears left and right blink latch so lamps stay in sync
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

    // Cruise set and reset are unused, use their signals for the lap counter
    if (local_crz_set && !last_crz_set_input) {
        if (lap_count < 999) {
            lap_count++;
        }
    }
    last_crz_set_input = local_crz_set;

    if (local_crz_reset && !last_crz_reset_input) {
        if (lap_count > 0) {
            lap_count--;
        }
    }
    last_crz_reset_input = local_crz_reset;

    portEXIT_CRITICAL(&stateMux);
}

void initIO() {
    pinMode(REGEN_BRAKE_PIN, INPUT);
    pinMode(HEADLIGHT_PIN, INPUT);
    pinMode(LEFT_BLINK_PIN, INPUT);
    pinMode(RIGHT_BLINK_PIN, INPUT);
    pinMode(DIRECTION_SWITCH_PIN, INPUT);
    pinMode(HORN_PIN, INPUT);
    pinMode(HAZARDS_PIN, INPUT);
    pinMode(DRIVE_MODE_PIN, INPUT);
    pinMode(CRZ_SET_PIN, INPUT);
    pinMode(CRZ_RESET_PIN, INPUT);

    last_headlight_input = digitalRead(HEADLIGHT_PIN);
    last_left_blink_input = digitalRead(LEFT_BLINK_PIN);
    last_right_blink_input = digitalRead(RIGHT_BLINK_PIN);
    last_direction_switch_input = digitalRead(DIRECTION_SWITCH_PIN);
    last_hazards_input = digitalRead(HAZARDS_PIN);
    last_drive_mode_input = digitalRead(DRIVE_MODE_PIN);
    last_crz_set_input = digitalRead(CRZ_SET_PIN);
    last_crz_reset_input = digitalRead(CRZ_RESET_PIN);

    // Default to forward on boot, first sampleIO() only seeds edge detection
    direction_switch_state = true;
    direction_input_armed = false;
    portENTER_CRITICAL(&stateMux);
    digital_data.direction_switch = true;
    portEXIT_CRITICAL(&stateMux);

    sampleIO();
}
