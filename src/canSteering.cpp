#include "canSteering.h"

bool send_success;
CANSteering::CANSteering(int8_t tx, int8_t rx, uint16_t tx_queue, uint16_t rx_queue, uint16_t frequency) : ESP32CANManager(tx, rx, tx_queue, rx_queue, frequency) {};

void CANSteering::readHandler(CanFrame msg) {

}

void CANSteering::sendSteeringData() {
    send_success = true;
    send_success &= this->sendMessage(0x300, (void*)&digital_data, sizeof(digital_data));
    send_success &= this->sendMessage(0x301, (void*)&regen_brake, sizeof(float));
}