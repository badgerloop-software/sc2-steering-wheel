// #include "canSteering.h"

// CANSteering::CANSteering(CAN_TypeDef* canPort, CAN_PINS pins, int frequency) : CANManager(canPort, pins, frequency) {};

// void CANSteering::readHandler(CAN_message_t msg) {
    
// }

// void CANSteering::sendSteeringData() {
//     this->sendMessage(0x300, (void*)&digital_data, sizeof(digital_data));
//     this->sendMessage(0x301, (void*)&regen_brake, sizeof(float));
// }
#include "canSteering.h"

#define MAX_ANALOG_VALUE 4095

bool send_success;
CANSteering::CANSteering(int8_t tx, int8_t rx, uint16_t tx_queue, uint16_t rx_queue, uint16_t frequency) : ESP32CANManager(tx, rx, tx_queue, rx_queue, frequency) {};

void CANSteering::readHandler(CanFrame msg) {

}

void CANSteering::sendSteeringData() {
    send_success = true;
    float regen_brake_calculation = 3.3 * regen_brake / MAX_ANALOG_VALUE;
    send_success &= this->sendMessage(0x300, (void*)&digital_data, sizeof(digital_data));
    send_success &= this->sendMessage(0x301, (void*)&regen_brake_calculation, sizeof(float));
}