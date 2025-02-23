#include "canSteering.h"

CANSteering::CANSteering(CAN_TypeDef* canPort, CAN_PINS pins, int frequency) : CANManager(canPort, pins, frequency) {};

void CANSteering::readHandler(CAN_message_t msg) {
    
}

void CANSteering::sendSteeringData() {

}