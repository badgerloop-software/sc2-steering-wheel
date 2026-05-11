#include <Arduino.h>
#include "canSteering.h"

#define MAX_ANALOG_VALUE 4095
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
    can_messages_read++;
    can_last_id = msg.identifier;
    can_last_dlc = msg.data_length_code;

    switch (msg.identifier){
        case 0x101:{
            // Pack State of Charge: 1 byte at index 4, 0.5% per count
            if (msg.data_length_code >= 5) {
                float soc = (float)msg.data[4] * 0.5f;
                if (soc > 100.0f) soc = 100.0f;
                battery_soc = soc;
            }
            break;
        }
        case 0x200:{
            if (msg.data_length_code > 0) {
                battery_fault_active = (msg.data[0] & 0x01U) != 0;
            }
            if (msg.data_length_code >= sizeof(float)) {
                memcpy(&stuff, msg.data, sizeof(float));
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
                memcpy(&speedsig, msg.data, sizeof(float));
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


}

void CANSteering::sendSteeringData() {
    send_success = true;
    float regen_brake_normalized = (float)regen_brake_percent / 100.0f;
    uint16_t throttle_raw = (throttle < 0.0f) ? 0U : (throttle > (float)MAX_ANALOG_VALUE ? MAX_ANALOG_VALUE : (uint16_t)throttle);
    uint8_t digital_payload = 0;

    digital_payload |= (digital_data.headlight ? 1U : 0U) << 0;
    digital_payload |= (digital_data.left_blink ? 1U : 0U) << 1;
    digital_payload |= (digital_data.right_blink ? 1U : 0U) << 2;
    digital_payload |= (digital_data.direction_switch ? 1U : 0U) << 3;
    digital_payload |= (digital_data.horn ? 1U : 0U) << 4;
    digital_payload |= (digital_data.crz_mode_a ? 1U : 0U) << 5;
    digital_payload |= (digital_data.crz_set ? 1U : 0U) << 6;
    digital_payload |= (digital_data.crz_reset ? 1U : 0U) << 7;

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

    tx_ok = this->sendMessage(0x303, (void*)&drive_mode, sizeof(uint8_t), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    tx_ok = this->sendMessage(0x304, (void*)&hazards, sizeof(bool), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;
}