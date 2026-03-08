#include "display.h"

extern TFT_eSPI tft;

void drawBatteryFault() {
    static uint32_t lastDraw = 0;

    if (millis() - lastDraw < 500) return;
    lastDraw = millis();

    int x = tft.width() - 80;
    int y = 10;

    // Clear region
    tft.fillRect(x - 5, y - 5, 70, 40, TFT_BLACK);

    // Draw battery body
    tft.drawRect(x, y, 50, 25, TFT_WHITE);

    // Battery positive terminal
    tft.fillRect(x + 50, y + 8, 6, 10, TFT_WHITE);

    // Draw fault X inside battery
    tft.drawLine(x + 10, y + 5, x + 40, y + 20, TFT_RED);
    tft.drawLine(x + 40, y + 5, x + 10, y + 20, TFT_RED);

    // Optional label
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(x - 5, y + 30);
    tft.print("FAULT");
}