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

    // sets pivot point at bottom left of screen
    pivotX = screenW / 4; 
    pivotY = screenH / 4;

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
void updatePointer(int speed) {
    // checks that value is between min and max values
    if (speed < MIN_SPEED) speed = MIN_SPEED;
    if (speed > MAX_SPEED) speed = MAX_SPEED;

    // converts to degrees and calls angle method
    double theta = 180.0 * speed / MAX_SPEED;
    updatePointerAngle(theta);
}