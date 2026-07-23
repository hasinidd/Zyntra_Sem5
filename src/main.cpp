#include <Arduino.h>
#include "temperature.h"
#include <Wire.h>          // ← add this line


void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22); // SDA=GPIO21, SCL=GPIO22

  Serial.println("=== Zyntra Temperature Test ===");

  // Initialise sensor
  if (!temperature_init()) {
    Serial.println("Temperature sensor failed. Check wiring.");
    while(1); // Stop here if sensor not found
  }

  Serial.println("Sensor ready. Reading temperature every 2 seconds.");
  Serial.println("Hold your wrist near the sensor.");
}

void loop() {
  float temp = temperature_read();
  Serial.print("[TEMP] Current: ");
  Serial.print(temp);
  Serial.println(" C");
  delay(2000);
}