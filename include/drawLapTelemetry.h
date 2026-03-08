#ifndef DRAW_LAP_TELEMETRY_H
#define DRAW_LAP_TELEMETRY_H

#include <Arduino.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

extern volatile uint32_t last_lap_update_ms;
extern volatile int32_t lap_count;
extern volatile int32_t current_section;
extern volatile uint32_t lap_duration;

void drawLapTelemetry();

#endif