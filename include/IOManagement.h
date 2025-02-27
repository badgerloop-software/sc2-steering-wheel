#ifndef __IO_MANAGEMENT_H__
#define __IO_MANAGEMENT_H__

#include <Arduino.h>

//Macros for pins
#define DIRECTION_SWITCH_PIN        PA_9
#define LEFT_BLINK_PIN              PB_1
#define RIGHT_BLINK_PIN             PF_0
#define CRZ_MODE_A_PIN              PF_1
#define CRZ_SET_PIN                 PA_8
#define CRZ_RESET_PIN               PB_5
#define HORN_PIN                    PA_7
#define REGEN_BRAKE_PIN             PA_4

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