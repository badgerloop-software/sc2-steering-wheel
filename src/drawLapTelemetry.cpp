#include "drawLapTelemetry.h"

void drawLapTelemetry() {

    static uint32_t lastDraw = 0;

    if (millis() - lastDraw < 500) return;
    lastDraw = millis();

    Serial.println("Drawing lap telemetry...");

    tft.fillRect(0, 0, tft.width(), 60, TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.print("Lap: "); 
    tft.println(lap_count);

    tft.setCursor(10, 30);
    tft.print("Section: "); 
    tft.println(current_section);

    tft.setCursor(10, 50);
    tft.print("Duration: "); 
    tft.println(lap_duration);
}