#ifndef DRAWIMAGE_H
#define DRAWIMAGE_H

#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

#include "BSRlogo.h"

#define HEIGHT 480
#define WIDTH 320

//Functions for Image renderer
bool drawImageFromBitmap();


#endif