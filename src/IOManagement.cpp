#include "IOManagement.h"

volatile Digital_Data digital_data;
volatile uint16_t regen_brake;
volatile uint16_t number_reads = 0;
hw_timer_t *io_timer = NULL;

void initIO() {
    // Initialize digital pins
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

    initADC();
}

void IRAM_ATTR readIO() {
    // Read analog input
    regen_brake = readADC(REGEN_BRAKE_PIN);
    throttle = readADC(THROTTLE_PIN);

    // Read digital inputs
    digital_data.headlight = digitalRead(HEADLIGHT_PIN);
    digital_data.left_blink = digitalRead(LEFT_BLINK_PIN);
    digital_data.right_blink = digitalRead(RIGHT_BLINK_PIN);
    digital_data.direction_switch = digitalRead(DIRECTION_SWITCH_PIN);
    digital_data.horn = digitalRead(HORN_PIN);
    digital_data.crz_mode_a = digitalRead(CRZ_MODE_A_PIN);
    digital_data.crz_set = digitalRead(CRZ_SET_PIN);
    digital_data.crz_reset = digitalRead(CRZ_RESET_PIN);

    hazards = digitalRead(HAZARDS_PIN);
    drive_mode = digitalRead(DRIVE_MODE_PIN);
    
    number_reads++;
}