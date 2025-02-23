#include "canSteering.h"

CANSteering::CANSteering(CAN_TypeDef* canPort, CAN_PINS pins, int frequency) : CANManager(canPort, pins, frequency) {};

void CANSteering::readHandler(CAN_message_t msg) {
    
}

void CANSteering::sendSteeringData() {
    this->sendMessage(0x300, (void*)&digital_data, sizeof(digital_data));
    this->sendMessage(0x301, (void*)&regen_brake, sizeof(float));
}