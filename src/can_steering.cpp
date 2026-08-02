#include "can_steering.h"

#include <Arduino.h>

#include "can_ids.h"
#include "debug.h"

// ------------- GLOBALS -------------

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

// ------------- LOCAL -------------

static volatile bool bps_src_505 = false;

// OCV curve: cell V at SOC 0,5,10,...,100 (matches streamlit app)
static const float CELL_VOLTAGES[] = {
    2.500f, 2.871f, 3.043f, 3.160f, 3.269f, 3.371f, 3.429f, 3.476f, 3.521f, 3.572f,
    3.629f, 3.680f, 3.726f, 3.765f, 3.804f, 3.865f, 3.923f, 3.955f, 3.975f, 3.998f, 4.111f,
};
static const int CELL_VOLTAGE_COUNT = sizeof(CELL_VOLTAGES) / sizeof(CELL_VOLTAGES[0]);

// ------------- LOCAL FUNCTIONS -------------

static inline uint16_t readBeUint16(const uint8_t* data) {
    return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

static float socFromPackVoltage(float pack_voltage) {
    float cell_voltage = pack_voltage / PACK_SERIES_CELLS;

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
    uint16_t magnitude = (raw_current >= SC2_CAN_BPS_PACK_CURRENT_ZERO)
                             ? (uint16_t)(raw_current - SC2_CAN_BPS_PACK_CURRENT_ZERO)
                             : (uint16_t)(SC2_CAN_BPS_PACK_CURRENT_ZERO - raw_current);
    return (float)magnitude * PACK_CURRENT_SCALE_A;
}

// ------------- PUBLIC FUNCTIONS -------------

CanSteering::CanSteering(int8_t tx, int8_t rx, uint16_t tx_queue, uint16_t rx_queue,
                         uint16_t frequency)
    : ESP32CANManager(tx, rx, tx_queue, rx_queue, frequency) {}

void CanSteering::readHandler(CanFrame msg) {
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

    switch (can_id) {
        case SC2_CAN_BPS_TEMPERATURE_ID:
            if (msg.data_length_code >= SC2_CAN_BPS_TEMPERATURE_DLC) {
                local_low_temp_c = (float)readBeUint16(&msg.data[0]);
                local_high_temp_c = (float)readBeUint16(&msg.data[2]);
                update_temps = true;
            }
            break;

        case SC2_CAN_BPS_ELECTRICAL_ID:
            if (msg.data_length_code >= SC2_CAN_BPS_ELECTRICAL_DLC) {
                local_high_cell_v = (float)readBeUint16(&msg.data[0]) * CELL_VOLTAGE_SCALE_V;
                local_low_cell_v = (float)readBeUint16(&msg.data[2]) * CELL_VOLTAGE_SCALE_V;
                local_pack_abs_current_a = decodePackAbsCurrentA(readBeUint16(&msg.data[4]));
                local_est_pack_v = ((local_high_cell_v + local_low_cell_v) / 2.0f) * PACK_SERIES_CELLS;
                local_battery_soc = socFromPackVoltage(local_est_pack_v);
                update_cell_pack = true;
            }
            break;

        case SC2_CAN_PDC_ACC_OUT_ID:
            if (msg.data_length_code >= sizeof(float)) {
                memcpy(&local_stuff, msg.data, sizeof(float));
                update_stuff = true;
            }
            break;

        case SC2_CAN_PDC_REGEN_ID:
            break;

        case SC2_CAN_PDC_MPH_ID:
            if (msg.data_length_code >= sizeof(float)) {
                memcpy(&local_speedsig, msg.data, sizeof(float));
                update_speedsig = true;
            }
            break;

        case SC2_CAN_STEERING_THROTTLE_ID:
            if (msg.data_length_code >= sizeof(uint16_t)) {
                uint16_t throttle_raw = 0;
                memcpy(&throttle_raw, msg.data, sizeof(uint16_t));

                float acc_in = 0.0f;
                if (throttle_raw <= THROTTLE_CAN_ADC_MIN) {
                    acc_in = 0.0f;
                } else if (throttle_raw >= THROTTLE_CAN_ADC_MAX) {
                    acc_in = 1.0f;
                } else {
                    acc_in = (float)(throttle_raw - THROTTLE_CAN_ADC_MIN) /
                             (float)(THROTTLE_CAN_ADC_MAX - THROTTLE_CAN_ADC_MIN);
                }

#if SC2_DEBUG
                Serial.printf("throttle RX: raw=%u normalized=%.3f\n", throttle_raw, acc_in);
#endif
            }
            break;

        case SC2_CAN_PT_FAULT_STATUS_ID:
            if (msg.data_length_code >= SC2_CAN_PT_FAULT_STATUS_DLC) {
                local_505 = (msg.data[0] & SC2_CAN_PT_FAULT_MASK) != 0;
                update_505 = true;
            }
            break;

        default:
            break;
    }

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
    battery_fault_active = bps_src_505;
    if (update_stuff) {
        stuff = local_stuff;
    }
    if (update_speedsig) {
        speedsig = local_speedsig;
    }
    portEXIT_CRITICAL(&stateMux);
}

void CanSteering::sendSteeringData() {
    this->runQueue(2);

    portENTER_CRITICAL(&stateMux);
    uint8_t local_regen_brake_percent = regen_brake_percent;
    float local_throttle = throttle;
    Digital_Data local_digital_data = const_cast<Digital_Data&>(digital_data);
    uint8_t local_drive_mode = drive_mode;
    bool local_hazards = hazards;
    bool local_battery_fault_active = battery_fault_active;
    portEXIT_CRITICAL(&stateMux);

    send_success = true;

    bool blink_phase = getBlinkPhase();
    bool left_lamp = (local_digital_data.left_blink || local_hazards) && blink_phase;
    bool right_lamp = (local_digital_data.right_blink || local_hazards) && blink_phase;
    float regen_brake_normalized = (float)local_regen_brake_percent / 100.0f;
    uint16_t throttle_raw =
        (local_throttle < 0.0f)
            ? 0U
            : (local_throttle > (float)THROTTLE_SENT_MAX ? THROTTLE_SENT_MAX
                                                         : (uint16_t)local_throttle);
    uint8_t digital_payload = 0;

    digital_payload |= (local_digital_data.headlight ? 1U : 0U) << 0;
    digital_payload |= (left_lamp ? 1U : 0U) << 1;
    digital_payload |= (right_lamp ? 1U : 0U) << 2;
    digital_payload |= (local_digital_data.direction_switch ? 1U : 0U) << 3;
    digital_payload |= (local_digital_data.horn ? 1U : 0U) << 4;

    bool tx_ok = this->sendMessage(SC2_CAN_STEERING_DIGITAL_ID, (void*)&digital_payload,
                                  sizeof(digital_payload), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    tx_ok = this->sendMessage(SC2_CAN_STEERING_REGEN_ID, (void*)&regen_brake_normalized,
                              sizeof(float), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    tx_ok = this->sendMessage(SC2_CAN_STEERING_THROTTLE_ID, (void*)&throttle_raw,
                              sizeof(throttle_raw), CAN_SEND_TIMEOUT_MS);
#if SC2_DEBUG
    if (!tx_ok) {
        Serial.printf("Failed to send throttle: raw=%u\n", throttle_raw);
    } else {
        Serial.printf("Sent throttle: raw=%u\n", throttle_raw);
    }
#endif
    send_success &= tx_ok;

    tx_ok = this->sendMessage(SC2_CAN_STEERING_DRIVE_MODE_ID, (void*)&local_drive_mode,
                              sizeof(uint8_t), CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    bool hazard_blink = local_hazards && blink_phase;
    tx_ok =
        this->sendMessage(SC2_CAN_STEERING_HAZARD_ID, (void*)&hazard_blink, sizeof(bool),
                          CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;

    uint8_t bps_light = local_battery_fault_active ? 1U : 0U;
    tx_ok = this->sendMessage(SC2_CAN_BPS_LIGHT_ID, (void*)&bps_light, sizeof(bps_light),
                              CAN_SEND_TIMEOUT_MS);
    send_success &= tx_ok;
}
