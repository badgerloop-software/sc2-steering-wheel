#include "drawImage.h"

// Create a TFT_eSPI object
TFT_eSPI tft = TFT_eSPI();

// Convert from 8-bit RRRGGGBB to 16-bit RGB565
uint16_t* convertToRGB565(const uint8_t* rrrgggbb_array, int size) {
    uint16_t* rgb565_array = (uint16_t*)malloc(size * sizeof(uint16_t));
    if (!rgb565_array) return NULL;
    
    for (int i = 0; i < size; i++) {
        uint8_t rrrgggbb = rrrgggbb_array[i];
        
        // Extract components
        uint8_t r = (rrrgggbb >> 5) & 0x07;  // Extract R (bits 5-7)
        uint8_t g = (rrrgggbb >> 2) & 0x07;  // Extract G (bits 2-4)
        uint8_t b = rrrgggbb & 0x03;         // Extract B (bits 0-1)
        
        // Scale to RGB565 bit depths
        uint16_t r565 = (r * 31) / 7;  // Scale 0-7 to 0-31 (5 bits)
        uint16_t g565 = (g * 63) / 7;  // Scale 0-7 to 0-63 (6 bits)
        uint16_t b565 = (b * 31) / 3;  // Scale 0-3 to 0-31 (5 bits)
        
        // Combine into RGB565 format
        rgb565_array[i] = (r565 << 11) | (g565 << 5) | b565;
    }
    
    return rgb565_array;
}

bool drawImageFromBitmap() {
    // Initialize the TFT_eSPI object
    tft.begin();

    // Set the background color to black
    tft.fillScreen(TFT_BLACK);

    // Convert from RRRGGGBB to RGB565
    int imageSize = 295 * 177;
    uint16_t* rgb565_data = convertToRGB565(BSRlogo, imageSize);
    if (!rgb565_data) return false;
    
    // Use the converted data
    tft.pushImage(WIDTH/2 - 295/2, HEIGHT/2 - 177/2, 177, 295, rgb565_data);
    
    // Free the memory
    free(rgb565_data);
    return true;
}