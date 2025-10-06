#ifndef __BRAIN_H__
#define __BRAIN_H__

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>

extern TFT_eSPI tft;

typedef enum {
    LINE,
    ARC,
    CIRCLE,
    RECTANGLE,
    TEXT,
    IMAGE,
} ObjectType;

typedef struct {
    int x;
    int y;
    int color;
    ObjectType ObjectID;
} DisplayObject;

#endif