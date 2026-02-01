#include <Arduino.h>
#include "canSteering.h"
#include "IOManagement.h"
#include "display.h"
#include "pointer.h"
#include "speedometer.h"

#define CAN_TX		21
#define CAN_RX		22

CANSteering canSteering(CAN_TX, CAN_RX, 10, 10, 250);
extern bool send_success;

extern float stuff;
extern float speedsig;

void setup() {
    Serial.begin(115200);
    initIO();

    begin();
    initSpeedometer();
}

void loop() {
    canSteering.sendSteeringData();

    canSteering.runQueue(CAN_QUEUE_PERIOD);

    float speed = speedsig;

    Serial.printf("Speed: %.2f\n", speed);

    
    updatePointer(speed);


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
    printf("throttle: %d\n", throttle);
    printf("hazards: %d\n", hazards);
    printf("drive_mode: %d\n", drive_mode);
    printf("number reads: %d\n", number_reads);
}
