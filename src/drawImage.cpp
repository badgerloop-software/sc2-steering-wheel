#include "drawImage.h"

// Create a TFT_eSPI object
TFT_eSPI tft = TFT_eSPI();

bool drawImageFromBitmap() {
    // Initialize the TFT_eSPI object
    tft.begin();

    // Set the background color to black
    tft.fillScreen(TFT_BLACK);
    // Create a temporary buffer
    uint16_t *rgb565_data = (uint16_t *)malloc(295 * 177 * sizeof(uint16_t));

    // Convert from uint8_t pairs to uint16_t
    for (int i = 0; i < 295 * 177; i++) {
        rgb565_data[i] = (uint16_t)BSRlogo[i*2] | ((uint16_t)BSRlogo[i*2+1] << 8);
    }

    // Use the converted data
    tft.pushImage(WIDTH/2, HEIGHT/2, 295, 177, rgb565_data);
    free(rgb565_data);
    return true;
}