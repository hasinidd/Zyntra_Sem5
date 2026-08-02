#include <Arduino.h>
#include <Wire.h>
#include "reaction_test.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22);

  Serial.println("=== Zyntra RT Test ===");
  rt_init();

  Serial.println("RT test will begin in 3 seconds.");
  Serial.println("Place finger near button — tap when you feel vibration.");
  delay(3000);
}

void loop() {
  uint16_t median = rt_run_test();

  Serial.print("=== TEST COMPLETE. Median: ");
  Serial.print(median);
  Serial.print("ms. Status: ");
  Serial.println(median < 500 ? "PASS" : "FAIL");

  Serial.println("Waiting 5 seconds before next test...");
  delay(5000);
}