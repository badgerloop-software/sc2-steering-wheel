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
  canSteering.sendSteeringData();
  canSteering.runQueue(CAN_QUEUE_PERIOD);

  float speed = speedsig;

  Serial.printf("Speed: %.2f\n", speed);

  Serial.printf("direction %d\n", digital_data.direction_switch);
  Serial.printf("crz set %d\n", digital_data.crz_set);

  
  updatePointer(speed);
}
