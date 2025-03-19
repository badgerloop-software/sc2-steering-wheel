#include <Arduino.h>
#include "canSteering.h"
#include "IOManagement.h"

#include "drawImage.h"

CANSteering canSteering(CAN1, DEF);

void setup() {
    Serial.begin(115200);
    Serial.println("Hello, World!!!!!!");
    drawImageFromBitmap();
    Serial.println("Hello, World!");
    initIO();
}

void loop() {
    canSteering.sendSteeringData();
    canSteering.runQueue(CAN_QUEUE_PERIOD);
}
