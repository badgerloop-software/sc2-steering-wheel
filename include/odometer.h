#ifndef __ODOMETER_H__
#define __ODOMETER_H__

#include <stdint.h>

// ------------- FUNCTIONS -------------

void initOdometer();
void updateOdometer(float speed_mph, uint32_t delta_ms);
uint32_t getOdometerMiles();
uint32_t getOdometerTenths();

#endif  // __ODOMETER_H__
