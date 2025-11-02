#include <Arduino.h>
#include "display.h"
#include "pointer.h"
#include "canSteering.h"

#define CAN_TX		21
#define CAN_RX		22

CANSteering canSteering(CAN_TX, CAN_RX, 10, 10, 250);
extern bool send_success;

extern float stuff;
extern float speedsig;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  initIO();
  begin(); // initialize pointer display
  
  // drawSdJpeg("/bsr/Jonathan.jpeg", 130, 0);
  // HeapAnim();
  Serial.println("Setup done.");
}

void loop() {
  // rotateColors();
  canSteering.runQueue(CAN_QUEUE_PERIOD);

  float speed = speedsig;

  Serial.printf("Speed: %.2f\n", speed);

  
  updatePointer(speed);



  // rotateColors();
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
