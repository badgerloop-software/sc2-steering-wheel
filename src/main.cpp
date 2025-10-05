#include <Arduino.h>
#include "display.h"
#include "pointer.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  initDisplay(false);
  
  // drawSdJpeg("/bsr/Jonathan.jpeg", 130, 0);
  // HeapAnim();
  Serial.println("Setup done.");
}

void loop() {
  rotateColors();
  // delay(42);
  // drawSdJpeg("/test.jpg", 0, 0);

  // HeapAnim();

  // pointer loop
  // if (Serial.available()) {
  //       String input = Serial.readStringUntil('\n');
  //       input.trim();

  //       if (input.startsWith("a")) {                          // ex: use a10 to set to 10 degrees
  //           double angle = input.substring(1).toDouble();
  //           updatePointerAngle(angle);
  //           Serial.printf("set to angle: %.2f deg\n", angle);
  //       } else {
  //           int value = input.toInt();                        // otherwise use integer method
  //           updatePointer(value);
  //           Serial.printf("set to value: %d\n", value);
  //       }
  //   }
}
