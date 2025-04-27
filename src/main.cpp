#include <Arduino.h>
#include "canSteering.h"
#include "IOManagement.h"

#define CAN_TX		21
#define CAN_RX		22

CANSteering canSteering(CAN_TX, CAN_RX, 10, 10, 250);
extern bool send_success;

void setup() {
    Serial.begin(115200);
    // initIO();
}

void loop() {
    canSteering.sendSteeringData();
    canSteering.runQueue(1000);
    printf("send_success: %d\n", send_success);
}
