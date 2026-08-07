// odometer: accumulate distance from mph and store tenths of a mile in NVS
#include "odometer.h"
#include <Preferences.h>

static Preferences prefs;
static uint32_t odoTenths = 0;      // tenths of a mile in flash
static double   accumulator = 0.0;  // fractional tenths not yet written

void initOdometer() {
    prefs.begin("odo", false);
    odoTenths = prefs.getUInt("tenths", 0);
}

void updateOdometer(float speedMph, uint32_t deltaMs) {
    if (speedMph <= 0.0f || deltaMs == 0) {
        return;
    }

    // miles = mph * ms / 3,600,000
    double distanceMiles = (double)speedMph * (double)deltaMs / 3600000.0;
    accumulator += distanceMiles * 10.0;

    // Write NVS only when at least one full tenth is complete
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
