#ifndef __IO_MANAGEMENT_H__
#define __IO_MANAGEMENT_H__


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