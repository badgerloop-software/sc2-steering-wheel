#include "IOManagement.h"

volatile Digital_Data digital_data;
volatile float regen_brake;
volatile uint8_t regen_brake_percent = 0;
volatile float throttle;
volatile uint16_t number_reads = 0;
volatile bool hazards = 0;
volatile uint8_t drive_mode = 0;
hw_timer_t *io_timer = NULL;
volatile bool io_read_pending = false;
static bool headlight_state = false;
static bool last_headlight_input = false;
static bool left_blink_state = false;
static bool last_left_blink_input = false;
static bool right_blink_state = false;
static bool last_right_blink_input = false;
static bool hazards_state = false;
static bool last_hazards_input = false;
static bool direction_switch_state = false;
static bool last_direction_switch_input = false;
static bool crz_mode_a_state = false;
static bool last_crz_mode_a_input = false;
static bool drive_mode_state = false;
static bool last_drive_mode_input = false;
static bool last_regen_button_input = false;

static const uint8_t REGEN_STEP_PERCENT = 5;
static const uint16_t MAX_ANALOG_VALUE = 4095;

static bool toggleOnPress(bool input, bool &state, bool &last_input) {
    if (input && !last_input) {
        state = !state;
    }

    last_input = input;
    return state;
}

static void sampleIO() {
    // Regen is a stepped button: each press increases 5%, wraps from 100% to 0%.
    bool regen_button_input = digitalRead(REGEN_BRAKE_PIN);
    if (regen_button_input && !last_regen_button_input) {
        regen_brake_percent = (regen_brake_percent >= 100)
                                 ? 0
                                 : (uint8_t)(regen_brake_percent + REGEN_STEP_PERCENT);
    }
    last_regen_button_input = regen_button_input;

    regen_brake = (float)((regen_brake_percent * MAX_ANALOG_VALUE) / 100);
    throttle = analogRead(THROTTLE_PIN);

    // Read digital inputs
    bool headlight_input = digitalRead(HEADLIGHT_PIN);
    digital_data.headlight = toggleOnPress(headlight_input, headlight_state, last_headlight_input);
    digital_data.left_blink = toggleOnPress(digitalRead(LEFT_BLINK_PIN), left_blink_state, last_left_blink_input);
    digital_data.right_blink = toggleOnPress(digitalRead(RIGHT_BLINK_PIN), right_blink_state, last_right_blink_input);
    digital_data.direction_switch = toggleOnPress(digitalRead(DIRECTION_SWITCH_PIN), direction_switch_state, last_direction_switch_input);
    digital_data.horn = digitalRead(HORN_PIN);
    digital_data.crz_mode_a = toggleOnPress(digitalRead(CRZ_MODE_A_PIN), crz_mode_a_state, last_crz_mode_a_input);
    digital_data.crz_set = digitalRead(CRZ_SET_PIN);
    digital_data.crz_reset = digitalRead(CRZ_RESET_PIN);

    hazards = toggleOnPress(digitalRead(HAZARDS_PIN), hazards_state, last_hazards_input);
    drive_mode = toggleOnPress(digitalRead(DRIVE_MODE_PIN), drive_mode_state, last_drive_mode_input);

    number_reads++;
}

void initIO() {
    // Initialize digital pins
    pinMode(REGEN_BRAKE_PIN, INPUT);
    pinMode(HEADLIGHT_PIN, INPUT);
    pinMode(LEFT_BLINK_PIN, INPUT);
    pinMode(RIGHT_BLINK_PIN, INPUT);
    pinMode(DIRECTION_SWITCH_PIN, INPUT);
    pinMode(HORN_PIN, INPUT);
    pinMode(CRZ_MODE_A_PIN, INPUT);
    pinMode(CRZ_SET_PIN, INPUT);
    pinMode(CRZ_RESET_PIN, INPUT);
    pinMode(HAZARDS_PIN, INPUT);
    pinMode(DRIVE_MODE_PIN, INPUT);

    // Initialize timer for reading inputs
    io_timer = timerBegin(0,  // which timer (choose between 0 and 3)
                            80, // prescaler
                            true // counts up
    );
    timerAttachInterrupt(io_timer, &readIO, true);
    timerAlarmWrite(io_timer, IO_UPDATE_PERIOD, true);
    timerAlarmEnable(io_timer); // start the timer

    // Seed inputs once at startup so values are valid before first timer tick.
    sampleIO();

    last_headlight_input = digitalRead(HEADLIGHT_PIN);
    last_left_blink_input = digitalRead(LEFT_BLINK_PIN);
    last_right_blink_input = digitalRead(RIGHT_BLINK_PIN);
    last_direction_switch_input = digitalRead(DIRECTION_SWITCH_PIN);
    last_crz_mode_a_input = digitalRead(CRZ_MODE_A_PIN);
    last_hazards_input = digitalRead(HAZARDS_PIN);
    last_drive_mode_input = digitalRead(DRIVE_MODE_PIN);
    last_regen_button_input = digitalRead(REGEN_BRAKE_PIN);
}

void IRAM_ATTR readIO() {
    io_read_pending = true;
}

void updateIO() {
    if (io_read_pending) {
        io_read_pending = false;
        sampleIO();
    }
}