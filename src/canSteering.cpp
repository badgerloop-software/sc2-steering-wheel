#include <Arduino.h>
#include "canSteering.h"

static const uint32_t CAN_SEND_TIMEOUT_MS = 10;

float stuff = 0.0;
float speedsig = 0.0;

bool send_success;
volatile uint32_t can_messages_read = 0;
volatile uint16_t can_last_id = 0;
volatile uint8_t can_last_dlc = 0;
volatile bool battery_fault_active = false;
volatile float battery_soc = 0.0f;

CANSteering::CANSteering(int8_t tx, int8_t rx, uint16_t tx_queue, uint16_t rx_queue, uint16_t frequency) : ESP32CANManager(tx, rx, tx_queue, rx_queue, frequency) {};
void CANSteering::readHandler(CanFrame msg) {
    // 1. Prepare local variables to update under spinlock
    uint16_t local_last_id = msg.identifier;
    uint8_t local_last_dlc = msg.data_length_code;

    float local_battery_soc = -1.0f;
    bool local_battery_fault_active = false;
    bool update_battery_fault = false;
    bool update_battery_soc = false;
    float local_stuff = 0.0f;
    bool update_stuff = false;
    float local_speedsig = 0.0f;
    bool update_speedsig = false;

    switch (msg.identifier){
        case 0x101:{
            // Pack State of Charge: 1 byte at index 4, 0.5% per count
            if (msg.data_length_code >= 5) {
                float soc = (float)msg.data[4] * 0.5f;
                if (soc > 100.0f) soc = 100.0f;
                local_battery_soc = soc;
                update_battery_soc = true;
            }
            break;
        }
        case 0x200:{
            if (msg.data_length_code > 0) {
                local_battery_fault_active = (msg.data[0] & 0x01U) != 0;
                update_battery_fault = true;
            }
            if (msg.data_length_code >= sizeof(float)) {
                memcpy(&local_stuff, msg.data, sizeof(float));
                update_stuff = true;
            }
            break;
        }

        case 0x201:{
            // 0x201 is regen_brake from PCD — not used on display currently
            break;
        }
        case 0x208:{
            // mph from PCD (matches canPDC.cpp sendMessage(0x208, &mph, ...))
            if (msg.data_length_code >= sizeof(float)) {
                memcpy(&local_speedsig, msg.data, sizeof(float));
                update_speedsig = true;
            }
            break;
        }
        case 0x302: {
            if (msg.data_length_code >= sizeof(uint16_t)) {
                uint16_t throttle_raw = 0;
                memcpy(&throttle_raw, msg.data, sizeof(uint16_t));

                const uint16_t ADC_MIN = 869;   // ~0.7V at rest
                const uint16_t ADC_MAX = 3228;  // ~2.6V at full press

                float acc_in = 0.0f;
                if (throttle_raw <= ADC_MIN) {
                    acc_in = 0.0f;
                } else if (throttle_raw >= ADC_MAX) {
                    acc_in = 1.0f;
                } else {
                    acc_in = (float)(throttle_raw - ADC_MIN) / (float)(ADC_MAX - ADC_MIN);
                }

#ifdef DEBUG_PRINTS
                Serial.printf("CAN 0x302: raw=%u normalized=%.3f\n", throttle_raw, acc_in);
#endif
            }
            break;
        }
        default:
            break;
    }

    // 2. Commit updates under the spinlock critical section
    portENTER_CRITICAL(&stateMux);
    can_messages_read++;
    can_last_id = local_last_id;
    can_last_dlc = local_last_dlc;
    if (update_battery_soc) {
        battery_soc = local_battery_soc;
    }
    if (update_battery_fault) {
        battery_fault_active = local_battery_fault_active;
    }
    if (update_stuff) {
        stuff = local_stuff;
    }
    if (update_speedsig) {
        speedsig = local_speedsig;
    }
    portEXIT_CRITICAL(&stateMux);
}

void CANSteering::sendSteeringData() {
    // 1. Copy shared volatile variables under critical section
    portENTER_CRITICAL(&stateMux);
    uint8_t local_regen_brake_percent = regen_brake_percent;
    float local_throttle = throttle;
    Digital_Data local_digital_data = const_cast<Digital_Data&>(digital_data);
    uint8_t local_drive_mode = drive_mode;
    bool local_hazards = hazards;
    portEXIT_CRITICAL(&stateMux);

    send_success = true;

    // Compute blink phase from steering wheel clock — all boards will be in sync
    bool blink_phase = getBlinkPhase();
    bool left_lamp = (local_digital_data.left_blink || local_hazards) && blink_phase;
    bool right_lamp = (local_digital_data.right_blink || local_hazards) && blink_phase;
    float regen_brake_normalized = (float)local_regen_brake_percent / 100.0f;
    uint16_t throttle_raw = (local_throttle < 0.0f) ? 0U
        : (local_throttle > (float)THROTTLE_SENT_MAX ? THROTTLE_SENT_MAX : (uint16_t)local_throttle);
    uint8_t digital_payload = 0;

    digital_payload |= (local_digital_data.headlight ? 1U : 0U) << 0;
    digital_payload |= (left_lamp ? 1U : 0U) << 1;
    digital_payload |= (right_lamp ? 1U : 0U) << 2;
    digital_payload |= (local_digital_data.direction_switch ? 1U : 0U) << 3;
    digital_payload |= (local_digital_data.horn ? 1U : 0U) << 4;

    bool tx_ok = this->sendMessage(0x300, (void*)&digital_payload, sizeof(digital_payload), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    tx_ok = this->sendMessage(0x301, (void*)&regen_brake_normalized, sizeof(float), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    tx_ok = this->sendMessage(0x302, (void*)&throttle_raw, sizeof(throttle_raw), CAN_SEND_TIMEOUT_MS);
#ifdef DEBUG_PRINTS
    if (!tx_ok) {
        Serial.printf("Failed to send CAN 0x302: raw=%u\n", throttle_raw);
    } else {
        Serial.printf("Sent CAN 0x302: raw=%u\n", throttle_raw);
    }
#endif
    send_success &= tx_ok;

    tx_ok = this->sendMessage(0x303, (void*)&local_drive_mode, sizeof(uint8_t), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    bool hazard_blink = local_hazards && blink_phase;
    tx_ok = this->sendMessage(0x304, (void*)&hazard_blink, sizeof(bool), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;
}