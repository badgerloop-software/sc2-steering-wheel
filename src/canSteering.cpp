#include <Arduino.h>
#include "canSteering.h"

static const uint32_t CAN_SEND_TIMEOUT_MS = 10;

float stuff = 0.0;
float speedsig = 0.0;

bool send_success;
volatile uint32_t can_messages_read = 0;
volatile uint16_t can_last_id = 0;
volatile uint8_t can_last_dlc = 0;
volatile bool battery_fault_active = false;
volatile float battery_soc = 0.0f;
volatile float battery_low_temp_c = 0.0f;
volatile float battery_high_temp_c = 0.0f;
volatile float battery_high_cell_v = 0.0f;
volatile float battery_low_cell_v = 0.0f;
volatile float battery_pack_abs_current_a = 0.0f;
volatile float battery_est_pack_v = 0.0f;

static volatile bool bps_src_505 = false;
// static volatile bool bps_src_506 = false;
// static volatile bool bps_src_507 = false;

static inline uint16_t readBeUint16(const uint8_t *data) {
    return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

// Matches sc2-powertrain BPS_PACK_CURRENT_* : midscale 0x8000 = 0 A, 0.1 A/bit
static const uint16_t PACK_CURRENT_ZERO_RAW = 0x8000U;
static const float PACK_CURRENT_SCALE_A = 0.1f;
static const float CELL_VOLTAGE_SCALE_V = 0.0001f;

// OCV curve: cell V at SOC 0,5,10,...,100 (matches streamlit app)
static const float CELL_VOLTAGES[] = {
    2.500f, 2.871f, 3.043f, 3.160f, 3.269f,
    3.371f, 3.429f, 3.476f, 3.521f, 3.572f,
    3.629f, 3.680f, 3.726f, 3.765f, 3.804f,
    3.865f, 3.923f, 3.955f, 3.975f, 3.998f, 4.111f
};
static const int CELL_VOLTAGE_COUNT = sizeof(CELL_VOLTAGES) / sizeof(CELL_VOLTAGES[0]);
static const float SERIES_COUNT = 29.0f;

static float socFromPackVoltage(float pack_voltage) {
    float cell_voltage = pack_voltage / SERIES_COUNT;

    if (cell_voltage <= CELL_VOLTAGES[0]) {
        return 0.0f;
    }
    if (cell_voltage >= CELL_VOLTAGES[CELL_VOLTAGE_COUNT - 1]) {
        return 100.0f;
    }

    for (int i = 0; i < CELL_VOLTAGE_COUNT - 1; i++) {
        float low_v = CELL_VOLTAGES[i];
        float high_v = CELL_VOLTAGES[i + 1];
        if (low_v <= cell_voltage && cell_voltage <= high_v) {
            float fraction = (cell_voltage - low_v) / (high_v - low_v);
            return (float)(i * 5) + fraction * 5.0f;
        }
    }

    return 0.0f;
}

static float decodePackAbsCurrentA(uint16_t raw_current) {
    uint16_t magnitude = (raw_current >= PACK_CURRENT_ZERO_RAW)
        ? (uint16_t)(raw_current - PACK_CURRENT_ZERO_RAW)
        : (uint16_t)(PACK_CURRENT_ZERO_RAW - raw_current);
    return (float)magnitude * PACK_CURRENT_SCALE_A;
}

CANSteering::CANSteering(int8_t tx, int8_t rx, uint16_t tx_queue, uint16_t rx_queue, uint16_t frequency) : ESP32CANManager(tx, rx, tx_queue, rx_queue, frequency) {};
void CANSteering::readHandler(CanFrame msg) {
    // 1. Prepare local variables to update under spinlock
    const uint32_t can_id = msg.identifier & 0x7FFu;
    uint16_t local_last_id = (uint16_t)can_id;
    uint8_t local_last_dlc = msg.data_length_code;

    float local_battery_soc = 0.0f;
    float local_low_temp_c = 0.0f;
    float local_high_temp_c = 0.0f;
    bool update_temps = false;
    float local_high_cell_v = 0.0f;
    float local_low_cell_v = 0.0f;
    float local_pack_abs_current_a = 0.0f;
    float local_est_pack_v = 0.0f;
    bool update_cell_pack = false;
    float local_stuff = 0.0f;
    bool update_stuff = false;
    float local_speedsig = 0.0f;
    bool update_speedsig = false;

    bool local_505 = false;
    bool update_505 = false;
    // bool local_506 = false;
    // bool update_506 = false;
    // bool local_507 = false;
    // bool update_507 = false;

    switch (can_id){
        case 0x108:{
            // Low/High Temperature: big-endian uint16, 1 C (matches powertrain)
            if (msg.data_length_code >= 4) {
                local_low_temp_c = (float)readBeUint16(&msg.data[0]);
                local_high_temp_c = (float)readBeUint16(&msg.data[2]);
                update_temps = true;
            }
            break;
        }
        case 0x109:{
            // High/Low cell V (0.0001 V), Pack Abs Current (0x8000 midscale, 0.1 A)
            if (msg.data_length_code >= 6) {
                local_high_cell_v = (float)readBeUint16(&msg.data[0]) * CELL_VOLTAGE_SCALE_V;
                local_low_cell_v = (float)readBeUint16(&msg.data[2]) * CELL_VOLTAGE_SCALE_V;
                local_pack_abs_current_a = decodePackAbsCurrentA(readBeUint16(&msg.data[4]));
                local_est_pack_v = ((local_high_cell_v + local_low_cell_v) / 2.0f) * SERIES_COUNT;
                local_battery_soc = socFromPackVoltage(local_est_pack_v);
                update_cell_pack = true;
            }
            break;
        }
        case 0x200:{
            if (msg.data_length_code >= sizeof(float)) {
                memcpy(&local_stuff, msg.data, sizeof(float));
                update_stuff = true;
            }
            break;
        }

        case 0x201:{
            // 0x201 is regen_brake from PCD — not used on display currently
            break;
        }
        case 0x208:{
            // mph from PCD (matches canPDC.cpp sendMessage(0x208, &mph, ...))
            if (msg.data_length_code >= sizeof(float)) {
                memcpy(&local_speedsig, msg.data, sizeof(float));
                update_speedsig = true;
            }
            break;
        }
        case 0x302: {
            if (msg.data_length_code >= sizeof(uint16_t)) {
                uint16_t throttle_raw = 0;
                memcpy(&throttle_raw, msg.data, sizeof(uint16_t));

                const uint16_t ADC_MIN = 869;   // ~0.7V at rest
                const uint16_t ADC_MAX = 3228;  // ~2.6V at full press

                float acc_in = 0.0f;
                if (throttle_raw <= ADC_MIN) {
                    acc_in = 0.0f;
                } else if (throttle_raw >= ADC_MAX) {
                    acc_in = 1.0f;
                } else {
                    acc_in = (float)(throttle_raw - ADC_MIN) / (float)(ADC_MAX - ADC_MIN);
                }

#ifdef DEBUG_PRINTS
                Serial.printf("CAN 0x302: raw=%u normalized=%.3f\n", throttle_raw, acc_in);
#endif
            }
            break;
        }
        case 0x505: {
            // Powertrain estop: 0 = fault, nonzero = OK
            if (msg.data_length_code > 0) {
                local_505 = msg.data[0] == 0x01;
                update_505 = true;
            }
            break;
        }
        // case 0x506: {
        //     if (msg.data_length_code > 0) {
        //         local_506 = msg.data[0] == 0;
        //         update_506 = true;
        //     }
        //     break;
        // }
        // case 0x507: {
        //     if (msg.data_length_code > 0) {
        //         local_507 = msg.data[0] == 0;
        //         update_507 = true;
        //     }
        //     break;
        // }
        default:
            break;
    }

    // 2. Commit updates under the spinlock critical section
    portENTER_CRITICAL(&stateMux);
    can_messages_read++;
    can_last_id = local_last_id;
    can_last_dlc = local_last_dlc;
    if (update_temps) {
        battery_low_temp_c = local_low_temp_c;
        battery_high_temp_c = local_high_temp_c;
    }
    if (update_cell_pack) {
        battery_high_cell_v = local_high_cell_v;
        battery_low_cell_v = local_low_cell_v;
        battery_pack_abs_current_a = local_pack_abs_current_a;
        battery_est_pack_v = local_est_pack_v;
        battery_soc = local_battery_soc;
    }
    if (update_505) {
        bps_src_505 = local_505;
    }
    // if (update_506) {
    //     bps_src_506 = local_506;
    // }
    // if (update_507) {
    //     bps_src_507 = local_507;
    // }
    battery_fault_active = bps_src_505 /* || bps_src_506 || bps_src_507 */;
    if (update_stuff) {
        stuff = local_stuff;
    }
    if (update_speedsig) {
        speedsig = local_speedsig;
    }
    portEXIT_CRITICAL(&stateMux);
}

void CANSteering::sendSteeringData() {
    // Drain pending RX before TX so BMS frames are less likely to overflow the queue
    this->runQueue(2);

    // 1. Copy shared volatile variables under critical section
    portENTER_CRITICAL(&stateMux);
    uint8_t local_regen_brake_percent = regen_brake_percent;
    float local_throttle = throttle;
    Digital_Data local_digital_data = const_cast<Digital_Data&>(digital_data);
    uint8_t local_drive_mode = drive_mode;
    bool local_hazards = hazards;
    bool local_battery_fault_active = battery_fault_active;
    portEXIT_CRITICAL(&stateMux);

    send_success = true;

    // Compute blink phase from steering wheel clock — all boards will be in sync
    bool blink_phase = getBlinkPhase();
    bool left_lamp = (local_digital_data.left_blink || local_hazards) && blink_phase;
    bool right_lamp = (local_digital_data.right_blink || local_hazards) && blink_phase;
    float regen_brake_normalized = (float)local_regen_brake_percent / 100.0f;
    uint16_t throttle_raw = (local_throttle < 0.0f) ? 0U
        : (local_throttle > (float)THROTTLE_SENT_MAX ? THROTTLE_SENT_MAX : (uint16_t)local_throttle);
    uint8_t digital_payload = 0;

    digital_payload |= (local_digital_data.headlight ? 1U : 0U) << 0;
    digital_payload |= (left_lamp ? 1U : 0U) << 1;
    digital_payload |= (right_lamp ? 1U : 0U) << 2;
    digital_payload |= (local_digital_data.direction_switch ? 1U : 0U) << 3;
    digital_payload |= (local_digital_data.horn ? 1U : 0U) << 4;

    bool tx_ok = this->sendMessage(0x300, (void*)&digital_payload, sizeof(digital_payload), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    tx_ok = this->sendMessage(0x301, (void*)&regen_brake_normalized, sizeof(float), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    tx_ok = this->sendMessage(0x302, (void*)&throttle_raw, sizeof(throttle_raw), CAN_SEND_TIMEOUT_MS);
#ifdef DEBUG_PRINTS
    if (!tx_ok) {
        Serial.printf("Failed to send CAN 0x302: raw=%u\n", throttle_raw);
    } else {
        Serial.printf("Sent CAN 0x302: raw=%u\n", throttle_raw);
    }
#endif
    send_success &= tx_ok;

    tx_ok = this->sendMessage(0x303, (void*)&local_drive_mode, sizeof(uint8_t), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    bool hazard_blink = local_hazards && blink_phase;
    tx_ok = this->sendMessage(0x304, (void*)&hazard_blink, sizeof(bool), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    // Lightings BPS fault: CAN 0x103 bit 0
    uint8_t bps_light = local_battery_fault_active ? 1U : 0U;
    tx_ok = this->sendMessage(0x103, (void*)&bps_light, sizeof(bps_light), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;
}
