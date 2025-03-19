#ifndef DRAWIMAGE_H
#define DRAWIMAGE_H

#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

#include "BSRlogo.h"

extern const uint8_t BSRlogo[];

#define HEIGHT 480
#define WIDTH 320

//Functions for Image renderer
uint16_t* convertToRGB565(const uint8_t* rrrgggbb_array, int size);
bool drawImageFromBitmap();


#endif