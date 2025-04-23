#ifndef __SD_CODE_H__
#define __SD_CODE_H__

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

extern const char* ANIM_FILEPATH;

void initSD();

uint8_t *initFrameHeap(int frameSize);

void freeFrameHeap(uint8_t *frameHeap);

void loadFrameIntoHeap(int startIndex, int heapSize, uint8_t *frameHeap);

#endif