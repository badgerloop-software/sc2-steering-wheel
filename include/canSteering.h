#ifndef __CAN_STEERING_H__
#define __CAN_STEERING_H__

#include <ESP32-TWAI-CAN.hpp>
#include "esp32canmanager.h"
#include "IOManagement.h"

#define BLINK_DELAY_MS       400

inline bool getBlinkPhase() {
    return ((millis() / BLINK_DELAY_MS) % 2) != 0;
}

class CANSteering : public ESP32CANManager {
    public:
        CANSteering(int8_t tx, int8_t rx, uint16_t tx_queue, uint16_t rx_queue, uint16_t frequency = DEFAULT_ESP32_CAN_FREQ);
        void readHandler(CanFrame msg);
        void sendSteeringData();
};


extern volatile float speed_mph;

extern volatile bool battery_fault_active;
extern volatile float battery_soc;
extern volatile float battery_low_temp_c;
extern volatile float battery_high_temp_c;
extern volatile float battery_high_cell_v;
extern volatile float battery_low_cell_v;
extern volatile float battery_pack_abs_current_a;
extern volatile float battery_est_pack_v;

#endif
