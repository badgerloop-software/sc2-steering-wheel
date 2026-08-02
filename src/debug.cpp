#include "debug.h"

#if SC2_DEBUG

#include <Arduino.h>

// ------------- PUBLIC FUNCTIONS -------------

void debugInit() {
    Serial.begin(115200);
}

void debugUpdate() {
    // add periodic steering prints here
}

void debugError(const char* msg) {
    Serial.println(msg);
}

#endif  // SC2_DEBUG
