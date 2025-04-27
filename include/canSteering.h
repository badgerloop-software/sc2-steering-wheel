#ifndef __CAN_STEERING_H__
#define __CAN_STEERING_H__

#include <ESP32-TWAI-CAN.hpp>
#include "esp32canmanager.h"
#include "IOManagement.h"

#define CAN_QUEUE_PERIOD     50

class CANSteering : public ESP32CANManager {
    public:
        CANSteering(int8_t tx, int8_t rx, uint16_t tx_queue, uint16_t rx_queue, uint16_t frequency = DEFAULT_ESP32_CAN_FREQ);
        void readHandler(CanFrame msg);
        void sendSteeringData();
};

#endif