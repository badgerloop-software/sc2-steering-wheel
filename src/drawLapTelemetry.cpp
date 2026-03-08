#include "drawLapTelemetry.h"

void drawLapTelemetry() {

    static uint32_t lastDraw = 0;

    if (millis() - lastDraw < 500) return;
    lastDraw = millis();

    tft.fillRect(0, 180, tft.width(), 60, TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.setCursor(10, 190);
    tft.print("Lap: "); 
    tft.println(lap_count);

    tft.setCursor(10, 210);
    tft.print("Section: "); 
    tft.println(current_section);

    tft.setCursor(10, 230);
    tft.print("Duration: "); 
    tft.println(lap_duration);
}