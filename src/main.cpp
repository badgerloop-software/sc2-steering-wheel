#include <Arduino.h>
#include "canSteering.h"
#include "IOManagement.h"

#define CAN_TX		21
#define CAN_RX		22

CANSteering canSteering(CAN_TX, CAN_RX, 10, 10, 250);
extern bool send_success;

void setup() {
    Serial.begin(115200);
    initIO();
}

void loop() {
    canSteering.sendSteeringData();
    canSteering.runQueue(1000);
    printf("\033[2J"); // clears the screen
    printf("regen brake: %f\n", 3.3 * regen_brake / 4095);
    printf("send_success: %d\n", send_success);
    printf("headlight: %d\n", digital_data.headlight);
    printf("left blink: %d\n", digital_data.left_blink);
    printf("right_blink: %d\n", digital_data.right_blink);
    printf("direction_switch: %d\n", digital_data.direction_switch);
    printf("horn: %d\n", digital_data.horn);
    printf("crzmodea: %d\n", digital_data.crz_mode_a);
    printf("crz_set: %d\n", digital_data.crz_set);
    printf("crz_reset: %d\n", digital_data.crz_reset);
    printf("number reads: %d\n", number_reads);
}
