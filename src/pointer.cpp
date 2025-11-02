#include "pointer.h"
#include <Arduino.h>
#include <math.h>

TFT_eSPI tft = TFT_eSPI();

static int pivotX = 0;                  // x coord of pivot point
static int pivotY = 0;                  // y coord of pivot point
static int length = POINTER_LENGTH;     // length of pointer
static double currentTheta = 0.0;       // current angle in degrees

// erases pointer by painting background colored line over old line
static void erasePointer() {
    int oldX = pivotX + length * cos(currentTheta * DEG_TO_RAD);
    int oldY = pivotY + length * sin(currentTheta * DEG_TO_RAD);
    tft.drawLine(pivotX, pivotY, oldX, oldY, BG_COLOR);
}

// initializes and sets up pointer
void begin() {
    tft.init();
    tft.setRotation(1);                 // needed for some reason (see display.cpp initDisplay)
    tft.fillScreen(BG_COLOR);           // fills screen with background color

    // pulls screen width and height
    int screenW = tft.width();
    int screenH = tft.height();

    // sets pivot point at middle of screen
    pivotX = screenW / 2; 
    pivotY = screenH / 2;

    // sets pointer to 0 degrees
    currentTheta = 0.0;
    updatePointerAngle(0.0);

    Serial.println("begin");
}

// updates pointer given an angle in degrees
void updatePointerAngle(double theta) {
    erasePointer();                                             // erases old point
    currentTheta = theta;                                       // sets new degree to input

    int newX =  pivotX + length * cos(theta * DEG_TO_RAD);      // calculates new x endpoint
    int newY = pivotY + length * sin(theta * DEG_TO_RAD);       // calculates new y endpoint

    tft.drawLine(pivotX, pivotY, newX, newY, POINTER_COLOR);    // draws new line
}

// updates pointer given a integer value defined in pointer.h
void updatePointer(int value) {
    // checks that value is between min and max values
    if (value < MIN_VALUE) value = MIN_VALUE;
    if (value > MAX_VALUE) value = MAX_VALUE;

    // converts to degrees and calls angle method
    double theta = 180.0 * value / MAX_VALUE;
    updatePointerAngle(theta);
}

// draw static dial/background for the pointer (ticks, pivot, etc.)
void drawPointerBackground() {
    // Ensure background is cleared
    tft.fillScreen(BG_COLOR);

    // radius for the tick marks
    int radius = length + 8;

    // draw ticks every 10 units (0..100) mapped to 0..180 degrees
    for (int v = MIN_VALUE; v <= MAX_VALUE; v += 10) {
        double theta = 180.0 * v / MAX_VALUE;
        int x1 = pivotX + (radius - 8) * cos(theta * DEG_TO_RAD);
        int y1 = pivotY + (radius - 8) * sin(theta * DEG_TO_RAD);
        int x2 = pivotX + (radius) * cos(theta * DEG_TO_RAD);
        int y2 = pivotY + (radius) * sin(theta * DEG_TO_RAD);
        tft.drawLine(x1, y1, x2, y2, TFT_WHITE);
    }

    // draw outer semicircle (optional)
    // approximate by drawing many points along arc
    for (int deg = 0; deg <= 180; deg += 2) {
        int x = pivotX + radius * cos(deg * DEG_TO_RAD);
        int y = pivotY + radius * sin(deg * DEG_TO_RAD);
        tft.drawPixel(x, y, TFT_WHITE);
    }

    // pivot center
    tft.fillCircle(pivotX, pivotY, 4, TFT_WHITE);

    // redraw current pointer on top of background
    updatePointerAngle(currentTheta);
}