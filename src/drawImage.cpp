#include "drawImage.h"

// Create a TFT_eSPI object
TFT_eSPI tft = TFT_eSPI();

//Array to hold 10 TFT color values
uint16_t colors[10] = {TFT_RED, TFT_BLUE, TFT_GREEN, TFT_CYAN, TFT_MAGENTA, TFT_YELLOW, TFT_WHITE, TFT_ORANGE, TFT_PINK, TFT_PURPLE};

volatile int old_start_x = 0;
volatile int old_start_y = 0;
volatile int old_width = 0;
volatile int old_height = 0;

void initDisp(){
    tft.begin();
    tft.fillScreen(TFT_BLACK);
}

void rotateColors(){
    static uint16_t color = 10;
    tft.fillScreen(colors[color]);
    color = (color + 1) % 10;
}

bool drawImageFromBitmap(image_t image_code) {
    // Clear the previous image
    tft.fillRect(old_start_x, old_start_y, old_width, old_height, TFT_BLACK);

    // Use the converted data
    switch(image_code)
    {
        case BSR_LOGO:
            tft.pushImage(WIDTH/2 - BSRlogo_WIDTH/2, HEIGHT/2 - BSRlogo_HEIGHT/2, BSRlogo_WIDTH, BSRlogo_HEIGHT  , BSRlogo, true);
            old_start_x = WIDTH/2 - BSRlogo_WIDTH/2;
            old_start_y = HEIGHT/2 - BSRlogo_HEIGHT/2;
            old_width = BSRlogo_WIDTH;
            old_height = BSRlogo_HEIGHT;
            break;
        case CAR_IMAGE:
            tft.pushImage(WIDTH/2 - kar_WIDTH/2, HEIGHT/2 - kar_HEIGHT/2, kar_WIDTH, kar_HEIGHT, kar, true);
            old_start_x = WIDTH/2 - kar_WIDTH/2;
            old_start_y = kar_HEIGHT/2 - 168/2;
            old_width = kar_WIDTH;
            old_height = kar_HEIGHT;
            break;
        default:
            return false;
    }  

    return true;
}