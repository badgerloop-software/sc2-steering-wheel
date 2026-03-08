#ifndef BATTERY_FAULT_H
#define BATTERY_FAULT_H

#include <Arduino.h>
#include <TFT_eSPI.h>

// TFT display object defined elsewhere
extern TFT_eSPI tft;

// Function that draws the battery fault icon
void drawBatteryFault();

#endif