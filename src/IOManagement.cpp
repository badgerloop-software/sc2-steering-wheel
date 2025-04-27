#include "IOManagement.h"

volatile Digital_Data digital_data = {0, 1, 0, 0, 1, 0, 0};
volatile float regen_brake = 100.0f;

ESP32Timer IOTimer(0);

void initIO() {
    // // Initialize digital pins
    // pinMode(DIRECTION_SWITCH_PIN, INPUT);
    // pinMode(LEFT_BLINK_PIN, INPUT);
    // pinMode(RIGHT_BLINK_PIN, INPUT);
    // pinMode(CRZ_MODE_A_PIN, INPUT);
    // pinMode(CRZ_SET_PIN, INPUT);
    // pinMode(CRZ_RESET_PIN, INPUT);
    // pinMode(HORN_PIN, INPUT);

    // Initialize analog pins

    // Initialize timer for reading inputs
    if (IOTimer.attachInterruptInterval(IO_UPDATE_PERIOD, readIO)) {
        printf("IO Timer started \n");
    } else {
        printf("Failed to start IO Timer \n");
    }
}

bool readIO(void * timerNo) {
    // // Read digital inputs
    // digital_data.direction_switch = digitalRead(DIRECTION_SWITCH_PIN);
    // digital_data.left_blink = digitalRead(LEFT_BLINK_PIN);
    // digital_data.right_blink = digitalRead(RIGHT_BLINK_PIN);
    // digital_data.crz_mode_a = digitalRead(CRZ_MODE_A_PIN);
    // digital_data.crz_set = digitalRead(CRZ_SET_PIN);
    // digital_data.crz_reset = digitalRead(CRZ_RESET_PIN);
    // digital_data.horn = digitalRead(HORN_PIN);

    // // Read analog input
    // regen_brake = readADC(REGEN_BRAKE_PIN);
    digital_data.direction_switch = !digital_data.direction_switch;
    digital_data.left_blink = !digital_data.left_blink;
    digital_data.right_blink = !digital_data.right_blink;
    digital_data.crz_mode_a = !digital_data.crz_mode_a;
    digital_data.crz_set = !digital_data.crz_set;
    digital_data.crz_reset = !digital_data.crz_reset;
    digital_data.horn = !digital_data.horn;

    regen_brake += 1.00;

    return true; // default return to comply with the library
}