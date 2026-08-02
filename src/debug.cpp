#include "debug.h"

#if SC2_DEBUG

#include <Arduino.h>

// ------------- PUBLIC FUNCTIONS -------------

void debugInit() {
    Serial.begin(115200);
}

void debugUpdate() {
    // reserved for periodic steering debug prints
}

void debugError(const char* msg) {
    Serial.println(msg);
}

#endif  // SC2_DEBUG
