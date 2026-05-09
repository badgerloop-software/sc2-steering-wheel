#include "odometer.h"
#include <Preferences.h>

static Preferences prefs;
static uint32_t odoTenths = 0;      // stored value: tenths of a mile
static double   accumulator = 0.0;  // fractional distance not yet committed

void initOdometer() {
    prefs.begin("odo", false);                   // namespace "odo", read-write
    odoTenths = prefs.getUInt("tenths", 0);      // default 0 on first boot
}

void updateOdometer(float speedMph, uint32_t deltaMs) {
    if (speedMph <= 0.0f || deltaMs == 0) {
        return;
    }

    // distance in miles = speed (mi/h) * time (ms) / 3,600,000 (ms/h)
    double distanceMiles = (double)speedMph * (double)deltaMs / 3600000.0;

    // convert to tenths-of-a-mile and add to accumulator
    accumulator += distanceMiles * 10.0;

    // flush whole tenths from the accumulator into NVS
    if (accumulator >= 1.0) {
        uint32_t wholeTenths = (uint32_t)accumulator;
        odoTenths += wholeTenths;
        accumulator -= (double)wholeTenths;
        prefs.putUInt("tenths", odoTenths);
    }
}

uint32_t getOdometerMiles() {
    return odoTenths / 10;
}

uint32_t getOdometerTenths() {
    return odoTenths;
}
