#ifndef DISPLAY_H
#define DISPLAY_H

#include <SPI.h>
#include <TFT_eSPI.h>

// Panel is 320 by 480
#define WIDTH 480

#ifndef TFT_CS
#define TFT_CS 15
#endif

void initDisplay();
void renderMinimalDisplay(float speed);

#endif
