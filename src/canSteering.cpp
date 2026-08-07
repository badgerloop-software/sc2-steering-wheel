// canSteering: CAN for the steering wheel
// TX: driver pack 0x300 to 0x304
// RX: BMS 0x108/0x109, mph 0x208 from PDC, BPS fault 0x001 from powertrain
#include <Arduino.h>
#include "canSteering.h"

static const uint32_t CAN_SEND_TIMEOUT_MS = 10;

// mph from PDC (CAN 0x208)
volatile float speed_mph = 0.0f;

volatile bool battery_fault_active = false;
volatile float battery_soc = 0.0f;
volatile float battery_low_temp_c = 0.0f;
volatile float battery_high_temp_c = 0.0f;
volatile float battery_high_cell_v = 0.0f;
volatile float battery_low_cell_v = 0.0f;
volatile float battery_pack_abs_current_a = 0.0f;
volatile float battery_est_pack_v = 0.0f;

static inline uint16_t readBeUint16(const uint8_t *data) {
    return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

// Matches sc2-powertrain BPS_PACK_CURRENT_* : midscale 0x8000 = 0 A, 0.1 A/bit
static const uint16_t PACK_CURRENT_ZERO_RAW = 0x8000U;
static const float PACK_CURRENT_SCALE_A = 0.1f;
static const float CELL_VOLTAGE_SCALE_V = 0.0001f;

// OCV curve: cell V at SOC 0, 5, 10, ..., 100 stolen from streamlit app
static const float CELL_VOLTAGES[] = {
    2.500f, 2.871f, 3.043f, 3.160f, 3.269f,
    3.371f, 3.429f, 3.476f, 3.521f, 3.572f,
    3.629f, 3.680f, 3.726f, 3.765f, 3.804f,
    3.865f, 3.923f, 3.955f, 3.975f, 3.998f, 4.111f
};
static const int CELL_VOLTAGE_COUNT = sizeof(CELL_VOLTAGES) / sizeof(CELL_VOLTAGES[0]);
static const float SERIES_COUNT = 29.0f;

static float socFromPackVoltage(float pack_voltage) {
    // Rough estimate SOC from OCV lookup table
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
    // Decode off the critical section, then copy into shared state under stateMux
    const uint32_t can_id = msg.identifier & 0x7FFu;

    float local_battery_soc = 0.0f;
    float local_low_temp_c = 0.0f;
    float local_high_temp_c = 0.0f;
    bool update_temps = false;
    float local_high_cell_v = 0.0f;
    float local_low_cell_v = 0.0f;
    float local_pack_abs_current_a = 0.0f;
    float local_est_pack_v = 0.0f;
    bool update_cell_pack = false;
    float local_speed_mph = 0.0f;
    bool update_speed = false;

    bool local_fault = false;
    bool update_fault = false;

    switch (can_id){
        case 0x001:{
            // Powertrain BPS/estop fault: data[0] == 0x01 means fault
            if (msg.data_length_code > 0) {
                local_fault = msg.data[0] == 0x01;
                update_fault = true;
            }
            break;
        }
        case 0x108:{
            // Low and high temperature: big-endian uint16, 1 C - Same as powertrain
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
        case 0x208:{
            if (msg.data_length_code >= sizeof(float)) {
                memcpy(&local_speed_mph, msg.data, sizeof(float));
                update_speed = true;
            }
            break;
        }
        default:
            break;
    }

    portENTER_CRITICAL(&stateMux);
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
    if (update_fault) {
        battery_fault_active = local_fault;
    }
    if (update_speed) {
        speed_mph = local_speed_mph;
    }
    portEXIT_CRITICAL(&stateMux);
}

void CANSteering::sendSteeringData() {
    // Read pending RX before TX so the BMS RX queue does not overflow
    this->runQueue(2);

    // Snapshot shared IO under mutex
    portENTER_CRITICAL(&stateMux);
    uint8_t local_regen_brake_percent = regen_brake_percent;
    float local_throttle = throttle;
    Digital_Data local_digital_data = const_cast<Digital_Data&>(digital_data); // work around volatile
    uint8_t local_drive_mode = drive_mode;
    bool local_hazards = hazards;
    portEXIT_CRITICAL(&stateMux);

    bool blink_phase = getBlinkPhase();
    bool left_lamp = (local_digital_data.left_blink || local_hazards) && blink_phase;
    bool right_lamp = (local_digital_data.right_blink || local_hazards) && blink_phase;
    float regen_brake_normalized = (float)local_regen_brake_percent / 100.0f;
    uint16_t throttle_raw = (local_throttle < 0.0f) ? 0U
        : (local_throttle > (float)THROTTLE_SENT_MAX ? THROTTLE_SENT_MAX : (uint16_t)local_throttle);
    uint8_t digital_payload = 0;

    // Bit pack for lighting and PDC: headlight, blinks, direction, horn
    digital_payload |= (local_digital_data.headlight ? 1U : 0U) << 0;
    digital_payload |= (left_lamp ? 1U : 0U) << 1;
    digital_payload |= (right_lamp ? 1U : 0U) << 2;
    digital_payload |= (local_digital_data.direction_switch ? 1U : 0U) << 3;
    digital_payload |= (local_digital_data.horn ? 1U : 0U) << 4;

    this->sendMessage(0x300, (void*)&digital_payload, sizeof(digital_payload), CAN_SEND_TIMEOUT_MS);
    this->sendMessage(0x301, (void*)&regen_brake_normalized, sizeof(float), CAN_SEND_TIMEOUT_MS);

    bool tx_ok = this->sendMessage(0x302, (void*)&throttle_raw, sizeof(throttle_raw), CAN_SEND_TIMEOUT_MS);
#ifdef DEBUG_PRINTS
    if (!tx_ok) {
        Serial.printf("Failed to send CAN 0x302: raw=%u\n", throttle_raw);
    } else {
        Serial.printf("Sent CAN 0x302: raw=%u\n", throttle_raw);
    }
#else
    (void)tx_ok;
#endif

    this->sendMessage(0x303, (void*)&local_drive_mode, sizeof(uint8_t), CAN_SEND_TIMEOUT_MS);

    bool hazard_blink = local_hazards && blink_phase;
    this->sendMessage(0x304, (void*)&hazard_blink, sizeof(bool), CAN_SEND_TIMEOUT_MS);
}
