#ifndef DISPLAY_H
#define DISPLAY_H

#include <SPI.h>

#include <FS.h>
#include <SD.h>

#include <TFT_eSPI.h>

#define HEIGHT 320
#define WIDTH 480


void initDisplay(bool SD_enable = true);
void renderMinimalDisplay(float speed);


#endif