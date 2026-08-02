#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include "board_config.h"

#define HEIGHT DISPLAY_HEIGHT
#define WIDTH DISPLAY_WIDTH

// ------------- FUNCTIONS -------------

void initDisplay(bool sd_enable = true);
void renderMinimalDisplay(float speed);

#endif  // __DISPLAY_H__
