#include <Arduino.h>
#include "canSteering.h"
#include "IOManagement.h"

#include "drawImage.h"

CANSteering canSteering(CAN1, DEF);

void setup() {
    Serial.begin(115200);
    initDisp();
    initIO();
}

void loop() {
    // rotateColors();
    // delay(1000);
    drawImageFromBitmap(BSR_LOGO);
    delay(500);
    canSteering.sendSteeringData();
    canSteering.runQueue(CAN_QUEUE_PERIOD);
    drawImageFromBitmap(CAR_IMAGE);
    delay(500);
}
