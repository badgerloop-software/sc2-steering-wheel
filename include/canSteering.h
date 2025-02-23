#ifndef __CAN_STEERING_H__
#define __CAN_STEERING_H__

#include "canmanager.h"
#include "IOManagement.h"

#define CAN_QUEUE_PERIOD     50
class CANSteering : public CANManager {
    public:
        CANSteering(CAN_TypeDef* canPort, CAN_PINS pins, int frequency = DEFAULT_CAN_FREQ);
        void readHandler(CAN_message_t msg);
        void sendSteeringData();
};

#endif