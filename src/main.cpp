#include <Arduino.h>

#include "board_config.h"
#include "can_steering.h"
#include "debug.h"
#include "display.h"
#include "io_management.h"
#include "odometer.h"

// ------------- LOCAL -------------

static CanSteering* can_steering = nullptr;

static void ioTask(void* pvParameters) {
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(IO_TASK_PERIOD_MS);
    while (true) {
        sampleIO();
        vTaskDelayUntil(&last_wake_time, period);
    }
}

static void canRxTask(void* pvParameters) {
    while (true) {
        if (can_steering != nullptr) {
            can_steering->runQueue(20);
        }
        vTaskDelay(pdMS_TO_TICKS(CAN_RX_TASK_PERIOD_MS));
    }
}

static void canTxTask(void* pvParameters) {
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(CAN_TX_TASK_PERIOD_MS);
    while (true) {
        if (can_steering != nullptr) {
            can_steering->sendSteeringData();
        }
        vTaskDelayUntil(&last_wake_time, period);
    }
}

static void displayTask(void* pvParameters) {
    uint32_t last_loop_ms = millis();
    while (true) {
        uint32_t now_loop = millis();
        uint32_t delta_ms = now_loop - last_loop_ms;
        last_loop_ms = now_loop;

        portENTER_CRITICAL(&stateMux);
        float speed = speedsig;
        portEXIT_CRITICAL(&stateMux);

        updateOdometer(speed, delta_ms);
        renderMinimalDisplay(speed);

        vTaskDelay(pdMS_TO_TICKS(DISPLAY_TASK_PERIOD_MS));
    }
}

// ------------- PUBLIC FUNCTIONS -------------

void setup() {
    debugInit();
    initIO();
    initDisplay(false);
    initOdometer();

    static CanSteering can_steering_instance(CAN_TX_PIN, CAN_RX_PIN, CAN_TX_QUEUE_SIZE,
                                             CAN_RX_QUEUE_SIZE, CAN_FREQUENCY_KHZ);
    can_steering = &can_steering_instance;

    xTaskCreatePinnedToCore(ioTask, "ioTask", 4096, nullptr, 3, nullptr, 0);
    xTaskCreatePinnedToCore(canRxTask, "canRxTask", 4096, nullptr, 4, nullptr, 0);
    xTaskCreatePinnedToCore(canTxTask, "canTxTask", 4096, nullptr, 4, nullptr, 0);
    xTaskCreatePinnedToCore(displayTask, "displayTask", 8192, nullptr, 1, nullptr, 1);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
