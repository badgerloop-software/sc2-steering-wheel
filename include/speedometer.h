#ifndef SPEED_ARC_H
#define SPEED_ARC_H

#include <TFT_eSPI.h>
#include <Arduino.h>

// Speedometer arc color and radius
#define SPEED_ARC_COLOR TFT_WHITE
#define SPEED_ARC_RADIUS 80

#define BG_COLOR TFT_BLACK // background color

extern TFT_eSPI tft;

void initSpeedometer();

#endif