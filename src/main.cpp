#include <Arduino.h>
#include "canSteering.h"
#include "IOManagement.h"
#include "display.h"
#include "odometer.h"

#define CAN_TX		21
#define CAN_RX		22

CANSteering* canSteering = nullptr;

extern float speedsig;

// Task implementations
void ioTask(void* pvParameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(10); // 10ms (100 Hz)
    while (true) {
        sampleIO();
        vTaskDelayUntil(&lastWakeTime, period);
    }
}

void canRxTask(void* pvParameters) {
    while (true) {
        if (canSteering != nullptr) {
            // Process TWAI RX queue for 10ms
            canSteering->runQueue(10);
        }
        vTaskDelay(pdMS_TO_TICKS(5)); // Yield CPU to other tasks
    }
}

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

void displayTask(void* pvParameters) {
    uint32_t lastLoopMs = millis();
    while (true) {
        uint32_t nowLoop = millis();
        uint32_t deltaMs = nowLoop - lastLoopMs;
        lastLoopMs = nowLoop;

        // Copy speedsig under critical section
        portENTER_CRITICAL(&stateMux);
        float speed = speedsig;
        portEXIT_CRITICAL(&stateMux);

        updateOdometer(speed, deltaMs);
        renderMinimalDisplay(speed);

        vTaskDelay(pdMS_TO_TICKS(50)); // 50ms (20 Hz)
    }
}

void setup() {
    Serial.begin(115200);
    initIO();
    initDisplay(false);
    initOdometer();

    static CANSteering canSteeringInstance(CAN_TX, CAN_RX, 10, 10, 250);
    canSteering = &canSteeringInstance;

    // Create and schedule tasks
    // Pinned to Core 0 (timing-critical control tasks)
    xTaskCreatePinnedToCore(
        ioTask,
        "ioTask",
        4096,
        nullptr,
        3,          // Priority
        nullptr,
        0           // Core ID
    );

    xTaskCreatePinnedToCore(
        canRxTask,
        "canRxTask",
        4096,
        nullptr,
        4,          // Higher Priority
        nullptr,
        0           // Core ID
    );

    xTaskCreatePinnedToCore(
        canTxTask,
        "canTxTask",
        4096,
        nullptr,
        4,          // Higher Priority
        nullptr,
        0           // Core ID
    );

    // Pinned to Core 1 (slower, non-critical drawing tasks)
    xTaskCreatePinnedToCore(
        displayTask,
        "displayTask",
        8192,       // Larger stack size for TFT drawing
        nullptr,
        1,          // Lower Priority
        nullptr,
        1           // Core ID
    );
}

void loop() {
    // FreeRTOS tasks run concurrently. loop() yields CPU.
    vTaskDelay(pdMS_TO_TICKS(1000));
}
