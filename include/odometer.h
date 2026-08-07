#ifndef ODOMETER_H
#define ODOMETER_H

#include <stdint.h>


void initOdometer();

// Add distance from speed in mph and elapsed time in ms
// Write NVS when a new tenth of a mile is complete
void updateOdometer(float speedMph, uint32_t deltaMs);

// Return whole miles from the stored tenths
uint32_t getOdometerMiles();

#endif
