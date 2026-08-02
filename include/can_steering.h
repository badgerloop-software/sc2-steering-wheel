#ifndef __CAN_STEERING_H__
#define __CAN_STEERING_H__

#include <ESP32-TWAI-CAN.hpp>

#include "esp32canmanager.h"
#include "io_management.h"

// ------------- FUNCTIONS -------------

inline bool getBlinkPhase() {
    return ((millis() / BLINK_DELAY_MS) % 2) != 0;
}

// ------------- CLASS -------------

// IDs / DLCs from embedded-pio/can_ids.h (CAN_*)
class CanSteering : public ESP32CANManager {
   public:
    CanSteering(int8_t tx, int8_t rx, uint16_t tx_queue, uint16_t rx_queue,
                uint16_t frequency = DEFAULT_ESP32_CAN_FREQ);
    void readHandler(CanFrame msg);
    void sendSteeringData();
};

// ------------- GLOBALS -------------

extern volatile uint32_t can_messages_read;
extern volatile uint16_t can_last_id;
extern volatile uint8_t can_last_dlc;
extern volatile bool battery_fault_active;
extern volatile float battery_soc;
extern volatile float battery_low_temp_c;
extern volatile float battery_high_temp_c;
extern volatile float battery_high_cell_v;
extern volatile float battery_low_cell_v;
extern volatile float battery_pack_abs_current_a;
extern volatile float battery_est_pack_v;
extern float speedsig;

#endif  // __CAN_STEERING_H__
