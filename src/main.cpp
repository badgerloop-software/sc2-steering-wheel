// sc2-steering-wheel main: ESP32 HMI with FreeRTOS tasks
// Core 0: IO sample, CAN RX, CAN TX - Core 1: display and odometer
// loop() only yields - Real work is in the tasks below
#include <Arduino.h>
#include "canSteering.h"
#include "IOManagement.h"
#include "display.h"
#include "odometer.h"

#define CAN_TX		21
#define CAN_RX		22

CANSteering* canSteering = nullptr;

// Sample pedals and buttons at 100 Hz into shared state (stateMux)
void ioTask(void* pvParameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(10); // 10ms (100 Hz)
    while (true) {
        sampleIO();
        vTaskDelayUntil(&lastWakeTime, period);
    }
}

// Drain BMS and telem RX
void canRxTask(void* pvParameters) {
    while (true) {
        if (canSteering != nullptr) {
            // Read RX often so high-rate BMS frames are not dropped
            canSteering->runQueue(20);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// Send driver inputs to PDC and lighting at 50 Hz
void canTxTask(void* pvParameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(20); // 20ms (50 Hz)
    while (true) {
        if (canSteering != nullptr) {
            canSteering->sendSteeringData();
        }
        vTaskDelayUntil(&lastWakeTime, period);
    }
}

// Update odometer from mph, then redraw the TFT at 20 Hz
void displayTask(void* pvParameters) {
    uint32_t lastLoopMs = millis();
    while (true) {
        uint32_t nowLoop = millis();
        uint32_t deltaMs = nowLoop - lastLoopMs;
        lastLoopMs = nowLoop;

        portENTER_CRITICAL(&stateMux);
        float speed = speed_mph;
        portEXIT_CRITICAL(&stateMux);

        updateOdometer(speed, deltaMs);
        renderMinimalDisplay(speed);

        vTaskDelay(pdMS_TO_TICKS(50)); // 50ms (20 Hz)
    }
}

void setup() {
    Serial.begin(115200);
    initIO();
    initDisplay();
    initOdometer();

    static CANSteering canSteeringInstance(CAN_TX, CAN_RX, 32, 64, 250);
    canSteering = &canSteeringInstance;

    // Core 0 runs control tasks/Core 1 runs the display
    xTaskCreatePinnedToCore(ioTask, "ioTask", 4096, nullptr, 3, nullptr, 0);
    xTaskCreatePinnedToCore(canRxTask, "canRxTask", 4096, nullptr, 4, nullptr, 0);
    xTaskCreatePinnedToCore(canTxTask, "canTxTask", 4096, nullptr, 4, nullptr, 0);
    xTaskCreatePinnedToCore(displayTask, "displayTask", 8192, nullptr, 1, nullptr, 1);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
