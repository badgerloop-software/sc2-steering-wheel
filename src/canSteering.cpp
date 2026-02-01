#include "canSteering.h"

#define MAX_ANALOG_VALUE 4095

float stuff = 0.0;
float speedsig = 0.0;

bool send_success;
CANSteering::CANSteering(int8_t tx, int8_t rx, uint16_t tx_queue, uint16_t rx_queue, uint16_t frequency) : ESP32CANManager(tx, rx, tx_queue, rx_queue, frequency) {};
void CANSteering::readHandler(CanFrame msg) {
    switch (msg.identifier){
        case 0x200:{
            stuff = *((float*)msg.data);
            break;
        }

        case 0x201:{
            speedsig = *((float*)msg.data);
            break;
        }
        default:
            break;
    }


}

void CANSteering::sendSteeringData() {
    send_success = true;
    float regen_brake_calculation = 3.3 * regen_brake / MAX_ANALOG_VALUE;
    send_success &= this->sendMessage(0x300, (void*)&digital_data, sizeof(digital_data));
    send_success &= this->sendMessage(0x301, (void*)&regen_brake_calculation, sizeof(float));
    send_success &= this->sendMessage(0x302, (void*)&throttle, sizeof(uint16_t));
    send_success &= this->sendMessage(0x303, (void*)&drive_mode, sizeof(uint8_t));
    send_success &= this->sendMessage(0x304, (void*)&hazards, sizeof(bool));
}