#ifndef __IO_MANAGEMENT_H__
#define __IO_MANAGEMENT_H__

#include <Arduino.h>

// Macros for pins
#define REGEN_BRAKE_PIN             36
#define HEADLIGHT_PIN               39
#define LEFT_BLINK_PIN              34
#define RIGHT_BLINK_PIN             35
#define DIRECTION_SWITCH_PIN        32
#define HORN_PIN                    33
#define CRZ_MODE_A_PIN              25
#define CRZ_SET_PIN                 26
#define CRZ_RESET_PIN               27


#define IO_UPDATE_PERIOD 100000 // us

struct Digital_Data {
    bool headlight : 1;             // input
    bool left_blink : 1;            // input
    bool right_blink : 1;           // input
    bool direction_switch : 1;      // input
    bool horn : 1;                  // input
    bool crz_mode_a : 1;            // input
    bool crz_set : 1;               // input
    bool crz_reset : 1;             // input
};

extern volatile Digital_Data digital_data;
extern volatile uint16_t regen_brake;
extern volatile uint16_t number_reads;

// initialize digital and analog pins, and timer to read pins
void initIO();

// ISR to read digital and analog inputs
void IRAM_ATTR readIO();

#endif