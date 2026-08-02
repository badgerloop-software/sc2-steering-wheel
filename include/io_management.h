#ifndef __IO_MANAGEMENT_H__
#define __IO_MANAGEMENT_H__

#include <Arduino.h>

#include "board_config.h"

// ------------- TYPES -------------

struct Digital_Data {
    bool headlight : 1;
    bool left_blink : 1;
    bool right_blink : 1;
    bool direction_switch : 1;
    bool horn : 1;
};

// ------------- GLOBALS -------------

extern volatile Digital_Data digital_data;
extern volatile float regen_brake;
extern volatile uint8_t regen_brake_percent;
extern volatile float throttle;
extern volatile bool hazards;
extern volatile uint8_t drive_mode;
extern volatile uint16_t number_reads;
extern volatile uint16_t lap_count;

extern portMUX_TYPE stateMux;

// ------------- FUNCTIONS -------------

static inline float calibratePedal(uint16_t raw_adc) {
    if (raw_adc <= THROTTLE_ADC_REST) {
        return 0.0f;
    }

    float normalized = (float)(raw_adc - THROTTLE_ADC_REST) /
                       (float)(THROTTLE_ADC_FULL - THROTTLE_ADC_REST);
    if (normalized > 1.0f) {
        normalized = 1.0f;
    }

    return normalized * (float)THROTTLE_ADC_MAX;
}

void initIO();
void sampleIO();

#endif  // __IO_MANAGEMENT_H__
