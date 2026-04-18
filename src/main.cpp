#include <Arduino.h>
#include "canSteering.h"
#include "IOManagement.h"
#include "display.h"

#define CAN_TX		21
#define CAN_RX		22

CANSteering canSteering(CAN_TX, CAN_RX, 10, 10, 250);
extern bool send_success;

extern float speedsig;
static const uint32_t SERIAL_PRINT_INTERVAL_MS = 1000;
static uint32_t lastSerialPrintMs = 0;

void setup() {
    Serial.begin(115200);
    initIO();
    initDisplay(false);
}

void loop() {
    updateIO();

    canSteering.sendSteeringData();

    canSteering.runQueue(CAN_QUEUE_PERIOD);

    float speed = speedsig;

    renderMinimalDisplay(speed);

    uint32_t now = millis();
    if (now - lastSerialPrintMs >= SERIAL_PRINT_INTERVAL_MS) {
        Serial.println();
        Serial.println("--- Steering Input Telemetry ---");
        Serial.printf("speed: %.2f\n", speed);
        Serial.printf("regen_raw: %.0f regen_v: %.3f\n", regen_brake, 3.3f * regen_brake / 4095.0f);
        Serial.printf("accel_raw: %.0f\n", throttle);
        Serial.printf("headlight: %d left_blink: %d right_blink: %d hazards: %d\n",
                      digital_data.headlight, digital_data.left_blink, digital_data.right_blink, hazards);
        Serial.printf("direction_switch: %d horn: %d drive_mode: %d\n",
                      digital_data.direction_switch, digital_data.horn, drive_mode);
        Serial.printf("crz_mode_a: %d crz_set: %d crz_reset: %d\n",
                      digital_data.crz_mode_a, digital_data.crz_set, digital_data.crz_reset);
        Serial.printf("send_success: %d number_reads: %u\n", send_success, number_reads);
        Serial.printf("can_messages_read: %lu can_last_id: 0x%03X can_last_dlc: %u\n",
                      can_messages_read, can_last_id, can_last_dlc);

        lastSerialPrintMs = now;
    }
}
