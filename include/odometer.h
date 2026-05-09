#ifndef ODOMETER_H
#define ODOMETER_H

#include <stdint.h>

/**
 * Load the odometer value from NVS flash.
 * Call once in setup().
 */
void initOdometer();

/**
 * Accumulate distance based on current speed and elapsed time.
 * Writes to NVS only when a new tenth-of-a-mile boundary is crossed.
 *
 * @param speedMph  Current speed in miles per hour.
 * @param deltaMs   Milliseconds elapsed since the last call.
 */
void updateOdometer(float speedMph, uint32_t deltaMs);

/**
 * @return Total accumulated distance in whole miles.
 */
uint32_t getOdometerMiles();

/**
 * @return Total accumulated distance in tenths of a mile.
 */
uint32_t getOdometerTenths();

#endif
