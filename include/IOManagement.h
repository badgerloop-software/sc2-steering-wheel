#ifndef __IO_MANAGEMENT_H__
#define __IO_MANAGEMENT_H__

#include <Arduino.h>

// Uncomment to enable Serial debug prints. Comment out for production
// to save CPU cycles and reduce latency.
#define DEBUG_PRINTS

// Macros for pins
#define REGEN_BRAKE_PIN 35
#define HEADLIGHT_PIN 16
#define LEFT_BLINK_PIN 34
#define RIGHT_BLINK_PIN 5
#define DIRECTION_SWITCH_PIN 32
#define HORN_PIN 33
#define THROTTLE_PIN 12
#define HAZARDS_PIN 14
#define DRIVE_MODE_PIN 4

#define IO_UPDATE_PERIOD 100000 // us

// Throttle pedal calibration (12-bit ADC counts)
#define THROTTLE_ADC_MAX  4095U
#define THROTTLE_ADC_REST 721U   // idle pedal position -> 0% output
#define THROTTLE_ADC_FULL 2078U  // full pedal press -> 100% output
#define THROTTLE_SENT_MAX THROTTLE_ADC_MAX

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

struct Digital_Data {
  bool headlight : 1;        // input
  bool left_blink : 1;       // input
  bool right_blink : 1;      // input
  bool direction_switch : 1; // input
  bool horn : 1;             // input
};

extern volatile Digital_Data digital_data;
extern volatile float regen_brake;
extern volatile uint8_t regen_brake_percent;
extern volatile float throttle;
extern volatile bool hazards;
extern volatile uint8_t drive_mode;
extern volatile uint16_t number_reads;

extern portMUX_TYPE stateMux;

// initialize digital and analog pins
void initIO();

// Function to sample inputs - to be called by our FreeRTOS task
void sampleIO();

#endif
