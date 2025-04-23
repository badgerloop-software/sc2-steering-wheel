#include "SDcode.h"

const char* ANIM_FILEPATH = "bsr/som.txt";

void printDirectory(File dir, int numTabs);

void initSD() {
    // Initialize the SD card
    if (!SD.begin(D3)) {
        Serial.println("SD card initialization failed!");
        return;
    }

    File root = SD.open("/bsr");
    

    printDirectory(root, 0);
    Serial.println("SD card initialized successfully.");
}

void printDirectory(File dir, int numTabs) {

    while (true) {
  
      File entry =  dir.openNextFile();
  
      if (! entry) {
  
        // no more files
  
        break;
  
      }
  
      for (uint8_t i = 0; i < numTabs; i++) {
  
        Serial.print('\t');
  
      }
  
      Serial.print(entry.name());
  
      if (entry.isDirectory()) {
  
        Serial.println("/");
  
        printDirectory(entry, numTabs + 1);
  
      } else {
  
        // files have sizes, directories do not
  
        Serial.print("\t\t");
  
        Serial.println(entry.size(), DEC);
  
      }
  
      entry.close();
  
    }
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
    File file = SD.open("som.txt", FILE_READ);
    if (SD.exists(ANIM_FILEPATH) == false) {
        Serial.println("Animation file does not exist!");
        return;
    }
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