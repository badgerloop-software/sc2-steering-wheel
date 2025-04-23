#ifndef DRAWIMAGE_H
#define DRAWIMAGE_H

#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

// #include "BSRlogo.h"
// #include "kar.h"
#include "animation.h"
#include "SDcode.h"

extern const uint8_t BSRlogo[];

typedef enum {
    BSR_LOGO,
    CAR_IMAGE,
    ANIMATION_1,
    JONATHAN,
}image_t;

#define HEIGHT 480
#define WIDTH 320

//Functions for Image renderer
void initDisp();
bool drawImageFromBitmap(image_t image_code, bool clear_old = true);
void rotateColors();
void clearScreen();
void playAnimation(bool clear_old = true);
void displayFrame(int frame);
void HeapAnim(bool clear_old = true);
void HeapDispFrame(int frameIndex);

void jpegRender(int xpos, int ypos);
void drawSdJpeg(const char *filename, int xpos, int ypos);

#endif