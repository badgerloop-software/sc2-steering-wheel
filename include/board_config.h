#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

// ------------- TIMING -------------
#define IO_UPDATE_PERIOD 100000  // us
#define IO_TASK_PERIOD_MS 10
#define CAN_RX_TASK_PERIOD_MS 1
#define CAN_TX_TASK_PERIOD_MS 20
#define DISPLAY_TASK_PERIOD_MS 50
#define CAN_SEND_TIMEOUT_MS 10
#define CAN_QUEUE_PERIOD 50
#define BLINK_DELAY_MS 400

// ------------- DEBUG -------------
// 0 off (not in flash) | 1 human
#define SC2_DEBUG 0

// ------------- CAN -------------
#define CAN_TX_PIN 21
#define CAN_RX_PIN 22
#define CAN_TX_QUEUE_SIZE 32
#define CAN_RX_QUEUE_SIZE 64
#define CAN_FREQUENCY_KHZ 250

// ------------- IO PINS -------------
#define REGEN_BRAKE_PIN 35
#define HEADLIGHT_PIN 16
#define LEFT_BLINK_PIN 34
#define RIGHT_BLINK_PIN 5
#define DIRECTION_SWITCH_PIN 32
#define HORN_PIN 33
#define THROTTLE_PIN 12
#define HAZARDS_PIN 14
#define DRIVE_MODE_PIN 4
#define CRZ_SET_PIN 26    // crz_inc, lap increment (cruise unused)
#define CRZ_RESET_PIN 27  // crz_dec, lap decrement (cruise unused)

// ------------- THROTTLE -------------
// 12-bit ADC counts
#define THROTTLE_ADC_MAX 4095U
#define THROTTLE_ADC_REST 721U   // idle pedal -> 0%
#define THROTTLE_ADC_FULL 2078U  // full pedal -> 100%
#define THROTTLE_SENT_MAX THROTTLE_ADC_MAX

// throttle RX normalization (matches PDC path)
#define THROTTLE_CAN_ADC_MIN 869   // ~0.7V at rest
#define THROTTLE_CAN_ADC_MAX 3228  // ~2.6V at full press

// ------------- BMS SOC -------------
#define PACK_SERIES_CELLS 29.0f
#define CELL_VOLTAGE_SCALE_V 0.0001f
#define PACK_CURRENT_SCALE_A 0.1f

// ------------- DISPLAY -------------
#define DISPLAY_HEIGHT 320
#define DISPLAY_WIDTH 480

#endif  // __BOARD_CONFIG_H__
