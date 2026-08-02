#include "odometer.h"

#include <Preferences.h>

// ------------- LOCAL -------------

static Preferences prefs;
static uint32_t odo_tenths = 0;
static double accumulator = 0.0;

// ------------- PUBLIC FUNCTIONS -------------

void initOdometer() {
    prefs.begin("odo", false);
    odo_tenths = prefs.getUInt("tenths", 0);
}

void updateOdometer(float speed_mph, uint32_t delta_ms) {
    if (speed_mph <= 0.0f || delta_ms == 0) {
        return;
    }

    double distance_miles = (double)speed_mph * (double)delta_ms / 3600000.0;
    accumulator += distance_miles * 10.0;

    if (accumulator >= 1.0) {
        uint32_t whole_tenths = (uint32_t)accumulator;
        odo_tenths += whole_tenths;
        accumulator -= (double)whole_tenths;
        prefs.putUInt("tenths", odo_tenths);
    }
}

uint32_t getOdometerMiles() {
    return odo_tenths / 10;
}

uint32_t getOdometerTenths() {
    return odo_tenths;
}
