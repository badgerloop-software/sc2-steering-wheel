#include <Arduino.h>
#include "canSteering.h"
#include "IOManagement.h"
#include "display.h"

#define CAN_TX		21
#define CAN_RX		22

CANSteering* canSteering = nullptr;

extern float speedsig;
static const uint32_t CAN_SEND_INTERVAL_MS = 20;
static uint32_t lastCanSendMs = 0;

void setup() {
    Serial.begin(115200);
    initIO();
    initDisplay(false);

    static CANSteering canSteeringInstance(CAN_TX, CAN_RX, 10, 10, 250);
    canSteering = &canSteeringInstance;
}

void loop() {
    if (canSteering == nullptr) {
        return;
    }

    updateIO();

    uint32_t now = millis();
    if (now - lastCanSendMs >= CAN_SEND_INTERVAL_MS) {
        canSteering->sendSteeringData();
        lastCanSendMs = now;
    }

    canSteering->runQueue(CAN_QUEUE_PERIOD);

    float speed = speedsig;

    renderMinimalDisplay(speed);
}
