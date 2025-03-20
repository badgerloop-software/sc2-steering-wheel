#ifndef DRAWIMAGE_H
#define DRAWIMAGE_H

#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

#include "BSRlogo.h"
#include "kar.h"

extern const uint8_t BSRlogo[];

typedef enum {
    BSR_LOGO,
    CAR_IMAGE
}image_t;

#define HEIGHT 480
#define WIDTH 320

//Functions for Image renderer
void initDisp();
bool drawImageFromBitmap(image_t image_code);
void rotateColors();


#endif