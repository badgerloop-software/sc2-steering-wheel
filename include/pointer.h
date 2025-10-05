#ifndef POINTER_H
#define POINTER_H

#include <TFT_eSPI.h>
#include <Arduino.h>

// Max/min value for integer input (0 = 0 degrees, 100 = 180 degrees)
#define MAX_VALUE 100
#define MIN_VALUE 0

// Pointer color and length
#define POINTER_COLOR TFT_RED
#define POINTER_LENGTH 60

#define BG_COLOR TFT_BLACK // background color

extern TFT_eSPI tft;

void begin();

void updatePointerAngle(double theta);

void updatePointer(int value);

#endif