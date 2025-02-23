#include <Arduino.h>
#include "canSteering.h"
#include "IOManagement.h"

CANSteering canSteering(CAN1, DEF);

void setup() {
    Serial.begin(115200);

    initIO();
}

void loop() {
    canSteering.sendSteeringData();
    canSteering.runQueue(CAN_QUEUE_PERIOD);
}
