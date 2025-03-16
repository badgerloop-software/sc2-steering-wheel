#ifndef __IO_MANAGEMENT_H__
#define __IO_MANAGEMENT_H__

#include <Arduino.h>
#include "STM32TimerInterrupt_Generic.h"
#include "adc.h"

//Macros for pins
#define DIRECTION_SWITCH_PIN        PB1
#define LEFT_BLINK_PIN              PA9
#define RIGHT_BLINK_PIN             PA10
#define CRZ_MODE_A_PIN              PB6
#define CRZ_SET_PIN                 PB5
#define CRZ_RESET_PIN               PB4
#define HORN_PIN                    PA0
#define REGEN_BRAKE_PIN             ADC_CHANNEL_6 // PA1

#define IO_UPDATE_PERIOD 100000 // us

struct Digital_Data {
    bool direction_switch : 1;      // input
    bool left_blink : 1;            // input
    bool right_blink : 1;           // input
    bool crz_mode_a : 1;            // input
    bool crz_set : 1;               // input
    bool crz_reset : 1;             // input
    bool horn : 1;                  // input
};

extern volatile Digital_Data digital_data;

extern volatile float regen_brake;

// initialize digital and analog pins
void initIO();

// read digital and analog inputs
void readIO();

#endif