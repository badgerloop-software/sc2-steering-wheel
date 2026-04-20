#include <Arduino.h>
#include "display.h"
#include "IOManagement.h"

#include <math.h>

TFT_eSPI tft = TFT_eSPI();

//Array of random colors to cycle through
uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA};

void rotateColors() {
  static uint8_t colorIndex = 0;
  tft.fillScreen(colors[colorIndex]);
  colorIndex = (colorIndex + 1) % (sizeof(colors) / sizeof(colors[0]));
}

void initDisplay(bool SD_enable){
    // Set all chip selects high to avoid bus contention during initialisation of each peripheral
    // digitalWrite(22, HIGH); // Touch controller chip select (if used)
    digitalWrite(15, HIGH); // TFT screen chip select
    // digitalWrite( 5, HIGH); // SD card chips select, must use GPIO 5 (ESP32 SS)

    tft.begin();
    tft.setRotation(1);  // Somehow important to have an odd rotation number
                          //Display does not work properly if you do not set this and initialise the SD card
    tft.fillScreen(TFT_BLACK);

    if (!SD_enable) {
      Serial.println("Display initialized without SD card.");
      return;
    }

    if (!SD.begin(5, tft.getSPIinstance())) {
      Serial.println("Card Mount Failed");
      return;
    }
    uint8_t cardType = SD.cardType();
  
    if (cardType == CARD_NONE) {
      Serial.println("No SD card attached");
      return;
    }
  
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
      Serial.println("MMC");
    } else if (cardType == CARD_SD) {
      Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }

    // // Check if frame files need to be created from text
    // bool needsConversion = false;
    // for (int i = 0; i < 14; i++) {
    //   sprintf(currentFramePath, FRAME_FILE_PATTERN, i);
    //   if (!SD.exists(currentFramePath)) {
    //     needsConversion = true;
    //     break;
    //   }
    // }

    // // Convert the text animation to individual binary frame files if needed
    // if (needsConversion && SD.exists("/bsr/som.txt")) {
    //   Serial.println("Creating individual binary frame files...");
    //   convertTextToBinaryFrames("/bsr/som.txt");
    // }
  
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
  
    // countAvailableFrames();
    // Serial.printf("Total frames available: %d\n", totalFrames);

    Serial.println("Initialization done.");
}

static void drawBoolLabel(int x, int y, const char *label, bool value) {
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(x, y);
  tft.print(label);
  tft.print(": ");
  tft.setTextColor(value ? TFT_GREEN : TFT_DARKGREY, TFT_BLACK);
  tft.print(value ? "ON" : "OFF");
}

static void drawStatusPill(int x, int y, int w, int h, const char *label, bool value, uint16_t color) {
  tft.drawRoundRect(x, y, w, h, 6, color);
  tft.fillRect(x + 1, y + 1, w - 2, h - 2, value ? color : TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(value ? TFT_BLACK : color, value ? color : TFT_BLACK);
  tft.setCursor(x + 8, y + 8);
  tft.print(label);
}

static void drawAccelBar(int x, int y, int w, int h, int percent) {
  tft.drawRoundRect(x, y, w, h, 8, TFT_DARKGREY);
  tft.fillRect(x + 2, y + 2, w - 4, h - 4, TFT_BLACK);

  int fillHeight = ((h - 4) * percent) / 100;
  int fillY = y + h - 2 - fillHeight;
  tft.fillRect(x + 2, fillY, w - 4, fillHeight, TFT_GREEN);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(x, y - 18);
  tft.print("ACCEL");
}

void renderMinimalDisplay(float speed) {
  static bool hasPreviousFrame = false;
  static int lastSpeedValue = -1;
  static int lastAccelPercent = -1;
  static int lastRegenPercent = -1;
  static bool lastHeadlight = false;
  static bool lastLeftBlink = false;
  static bool lastRightBlink = false;
  static bool lastHazards = false;
  static bool lastCruiseMode = false;
  static bool lastCruiseSet = false;
  static bool lastCruiseReset = false;
  static uint8_t lastDriveMode = 255;
  static bool lastDirectionSwitch = false;

  int speedValue = (int)lroundf(speed);
  if (speedValue < 0) {
    speedValue = 0;
  }

  float accelVoltage = 3.3f * throttle / 4095.0f;
  if (accelVoltage < 0.618f) {
    accelVoltage = 0.618f;
  } else if (accelVoltage > 2.814f) {
    accelVoltage = 2.814f;
  }

  int accelPercent = (int)lroundf(((accelVoltage - 0.618f) / (2.814f - 0.618f)) * 100.0f);
  if (accelPercent < 0) {
    accelPercent = 0;
  } else if (accelPercent > 100) {
    accelPercent = 100;
  }

  int regenPercent = regen_brake_percent;
  if (regenPercent < 0) {
    regenPercent = 0;
  } else if (regenPercent > 100) {
    regenPercent = 100;
  }

  bool headlight = digital_data.headlight;
  bool leftBlink = digital_data.left_blink;
  bool rightBlink = digital_data.right_blink;
  bool hazardState = hazards;
  bool cruiseMode = digital_data.crz_mode_a;
  bool cruiseSet = digital_data.crz_set;
  bool cruiseReset = digital_data.crz_reset;
  uint8_t currentDriveMode = drive_mode;
  bool directionSwitch = digital_data.direction_switch;

  bool screenNeedsUpdate = !hasPreviousFrame ||
                           speedValue != lastSpeedValue ||
                           accelPercent != lastAccelPercent ||
                           regenPercent != lastRegenPercent ||
                           headlight != lastHeadlight ||
                           leftBlink != lastLeftBlink ||
                           rightBlink != lastRightBlink ||
                           hazardState != lastHazards ||
                           cruiseMode != lastCruiseMode ||
                           cruiseSet != lastCruiseSet ||
                           cruiseReset != lastCruiseReset ||
                           currentDriveMode != lastDriveMode ||
                           directionSwitch != lastDirectionSwitch;

  if (!screenNeedsUpdate) {
    return;
  }
  hasPreviousFrame = true;
  lastSpeedValue = speedValue;
  lastAccelPercent = accelPercent;
  lastRegenPercent = regenPercent;
  lastHeadlight = headlight;
  lastLeftBlink = leftBlink;
  lastRightBlink = rightBlink;
  lastHazards = hazardState;
  lastCruiseMode = cruiseMode;
  lastCruiseSet = cruiseSet;
  lastCruiseReset = cruiseReset;
  lastDriveMode = currentDriveMode;
  lastDirectionSwitch = directionSwitch;

  const char *modeText = currentDriveMode ? "PWR" : "ECO";
  const char *directionText = directionSwitch ? "REV" : "FWD";

  tft.fillScreen(TFT_BLACK);

  tft.drawFastHLine(0, 38, WIDTH, TFT_DARKGREY);
  tft.drawFastHLine(0, 254, WIDTH, TFT_DARKGREY);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(12, 10);
  tft.print("SPEED");

  drawStatusPill(128, 6, 82, 28, modeText, currentDriveMode, currentDriveMode ? TFT_RED : TFT_GREEN);
  drawStatusPill(220, 6, 82, 28, directionText, directionSwitch, TFT_CYAN);
  drawStatusPill(312, 6, 82, 28, "CRZ", cruiseMode || cruiseSet || cruiseReset, TFT_YELLOW);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(7);
  tft.setCursor(10, 60);
  char odoBuffer[8];
  snprintf(odoBuffer, sizeof(odoBuffer), "%d", speedValue);
  tft.print(odoBuffer);

  tft.setTextSize(2);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(12, 120);
  tft.print("MPH");

  char speedBuffer[8];
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(7);
  tft.setCursor((tft.width()/2) - (tft.textWidth(speedBuffer)/2), 60);
  snprintf(speedBuffer, sizeof(speedBuffer), "%d", speedValue);
  tft.print(speedBuffer);

  tft.setTextSize(2);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor((tft.width() - tft.textWidth("MPH"))/2, 120);
  tft.print("MPH");

  drawAccelBar(360, 58, 88, 170, accelPercent);

  tft.setTextSize(3);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(10, 150);
  tft.print("ACC ");
  tft.print(accelPercent);
  tft.print("%");

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(10, 176);
  tft.print("REGEN ");
  tft.print(regenPercent);
  tft.print("%");

  drawBoolLabel(10, 204, "HEADLIGHT", headlight);
  drawBoolLabel(10, 232, "LEFT BLINK", leftBlink);
  drawBoolLabel(10, 260, "RIGHT BLINK", rightBlink);
  drawBoolLabel(10, 288, "HAZARDS", hazardState);

  drawBoolLabel(220, 188, "CRZ A", cruiseMode);
  drawBoolLabel(220, 216, "CRZ SET", cruiseSet);
  drawBoolLabel(220, 244, "CRZ RST", cruiseReset);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(220, 276);
  tft.print("MODE: ");
  tft.setTextColor(currentDriveMode ? TFT_RED : TFT_GREEN, TFT_BLACK);
  tft.print(modeText);

  tft.setCursor(330, 276);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("DIR: ");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.print(directionText);
}