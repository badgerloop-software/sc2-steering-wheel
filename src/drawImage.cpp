#include "drawImage.h"

// Create a TFT_eSPI object
TFT_eSPI tft = TFT_eSPI();

bool drawImageFromBitmap() {
    // Initialize the TFT_eSPI object
    tft.begin();

    // Set the background color to black
    tft.fillScreen(TFT_BLACK);
    // Create a temporary buffer

    // Use the converted data
    tft.pushImage(0, 0, 295, 177, BSRlogo, true);
    return true;
}