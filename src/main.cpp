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
  delay(1000); // Wait for serial to initialize

  Serial.println("Starting...");
  initIO();
  begin(); // initialize pointer display
  
  CanFrame testBPS;
  testBPS.identifier = 0x100;
  testBPS.data_length_code = 2;
  testBPS.data[0] = 0x01; // Simulate BMS FAILSAFE ACTIVE
  testBPS.data[1] = 0x00;
  canSteering.readHandler(testBPS);


  CanFrame testSpeed;
  testSpeed.identifier = 0x201;
  testSpeed.data_length_code = 4;
  float fakeSpeed = 75.5f;
  memcpy(testSpeed.data, &fakeSpeed, 4);
  canSteering.readHandler(testSpeed);
  
  // drawSdJpeg("/bsr/Jonathan.jpeg", 130, 0);
  // HeapAnim();
  Serial.println("Manual Injection Complete. BPS should be YES, Speed should be 75.5");
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
  drawBatteryFault(bps_fault);

  static unsigned long lastPrint = 0;
  if(millis() - lastPrint > 500) {
    Serial.printf("LIVE BUS SPEED:%.2f | BPS Fault: %s\n", speed, bps_fault ? "YES" : "NO");
    lastPrint = millis();
  }

  
}


