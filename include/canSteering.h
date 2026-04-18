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

extern volatile uint32_t can_messages_read;
extern volatile uint16_t can_last_id;
extern volatile uint8_t can_last_dlc;
extern volatile bool battery_fault_active;

#endif