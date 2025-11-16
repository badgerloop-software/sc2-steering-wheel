#include "pointer.h"
#include "speedometer.h"
#include <Arduino.h>
#include <math.h>

int lowAngle = MIN_SPEED + 270;
int highAngle = MAX_SPEED - 10;
// Starts up the spedometer arc
void initSpeedometer() {
    // Initializes the pointer for the speedometer
        
    //TFT_eSPI tft = TFT_eSPI();
    int sar = SPEED_ARC_RADIUS;
    int sx = tft.width() / 2;
    int sy = tft.height() / 2;
    // Serial.printf("sx: %d\n ", sx);
    // Serial.printf("sy: %d\n ", sy);
   
    tft.drawSmoothArc(sx, sy, sar, sar, lowAngle, highAngle, SPEED_ARC_COLOR, BG_COLOR, false);

    // Adds a tick mark every 20 degrees, 15 pixels long
    // Also adds one in between each 20 degree segment, 10 pixels long 
    for (int i = 0; i <= 180; i += 20) {
        float radian = radians(i);
        tft.drawLine(sx + sar * cos(radian), 
                     sy + sar * sin(radian), 
                     sx + (sar - 15) * cos(radian), 
                     sy + (sar - 15) * sin(radian), 
                     SPEED_ARC_COLOR);
        radian = radians(i + 10);
        if (i >= 180){
            break;
        }
        tft.drawLine(sx + sar * cos(radian), 
                        sy + sar * sin(radian), 
                        sx + (sar - 10) * cos(radian), 
                        sy + (sar - 10) * sin(radian), 
                        SPEED_ARC_COLOR); 
    }
}