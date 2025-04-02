#include "SDcode.h"

const char* ANIM_FILEPATH = "/bsr/animation_array.txt";

void initSD(SPIClass spi) {
    // Initialize the SD card
    if (!SD.begin(A3)) {
        Serial.println("SD card initialization failed!");
        return;
    }
    Serial.println("SD card initialized successfully.");
}

uint8_t *initFrameHeap(int frameSize) {
    // Allocate memory for the frame heap
    uint8_t *frameHeap = (uint8_t *)malloc(frameSize * sizeof(uint8_t));
    if (frameHeap == NULL) {
        Serial.println("Failed to allocate memory for frame heap");
        return NULL;
    }
    return frameHeap;
}  

void freeFrameHeap(uint8_t *frameHeap) {
    // Free the allocated memory for the frame heap
    if (frameHeap != NULL) {
        free(frameHeap);
        Serial.println("Frame heap memory freed successfully");
    } else {
        Serial.println("Frame heap is already NULL, no need to free memory");
    }
}

void loadFrameIntoHeap(int startIndex, int heapSize, uint8_t *frameHeap) {
    // Open the SD card file for reading
    File file = SD.open(ANIM_FILEPATH);
    if (!file) {
        Serial.println("Failed to open animation file");
        return;
    }

    // Read the file line by line and store the values in the frame heap
    int index = 0;
    while (file.available()) {
        String line = file.readStringUntil('\n');
        if (index >= startIndex && index < startIndex + heapSize) {
            frameHeap[index - startIndex] = (uint8_t)line.toInt();
        }
        if (index >= startIndex + heapSize) {
            break; // Stop reading if we have filled the heap
        }
        index++;
    }

    // Close the file
    file.close();
}