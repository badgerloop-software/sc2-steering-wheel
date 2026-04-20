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

CANSteering::CANSteering(int8_t tx, int8_t rx, uint16_t tx_queue, uint16_t rx_queue, uint16_t frequency) : ESP32CANManager(tx, rx, tx_queue, rx_queue, frequency) {};
void CANSteering::readHandler(CanFrame msg) {
    can_messages_read++;
    can_last_id = msg.identifier;
    can_last_dlc = msg.data_length_code;

    switch (msg.identifier){
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
            if (msg.data_length_code >= sizeof(float)) {
                memcpy(&speedsig, msg.data, sizeof(float));
            }
            break;
        }
        default:
            break;
    }


}

void CANSteering::sendSteeringData() {
    send_success = true;
    float regen_brake_calculation = 3.3 * regen_brake / MAX_ANALOG_VALUE;

    bool tx_ok = this->sendMessage(0x300, (void*)&digital_data, sizeof(digital_data), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    tx_ok = this->sendMessage(0x301, (void*)&regen_brake_calculation, sizeof(float), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    tx_ok = this->sendMessage(0x302, (void*)&throttle, sizeof(uint16_t), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    tx_ok = this->sendMessage(0x303, (void*)&drive_mode, sizeof(uint8_t), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    tx_ok = this->sendMessage(0x304, (void*)&hazards, sizeof(bool), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;
}