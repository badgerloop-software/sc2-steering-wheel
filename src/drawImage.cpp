#include "drawImage.h"

// Create a TFT_eSPI object
TFT_eSPI tft = TFT_eSPI();

// Convert from 8-bit RRRGGGBB to 16-bit RGB565
uint16_t* convertToRGB565(int size) {
    uint16_t* rgb565_data = (uint16_t*)malloc(size * sizeof(uint16_t));
    if (!rgb565_data) return NULL;
    
    for (int i = 0; i < size; i++) {
        rgb565_data[i] = tft.color8to16(BSRlogo[i]);
    }
    return rgb565_data;
}

bool drawImageFromBitmap() {
    // Initialize the TFT_eSPI object
    tft.begin();

    // Set the background color to black
    tft.fillScreen(TFT_BLACK);
    // Create a temporary buffer

    uint16_t *rgb565_img = convertToRGB565(177 * 295);

    // Use the converted data
    tft.pushImage(WIDTH/2 - 295/2, HEIGHT/2 - 177/2, 295, 177, rgb565_img);
    free(rgb565_img);
    return true;
}