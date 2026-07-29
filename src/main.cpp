#include <Arduino.h>
#include <Wire.h>
#include "oled_display.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22);

  Serial.println("=== Zyntra OLED Test ===");

  if (!oled_init()) {
    Serial.println("OLED failed. Check wiring.");
    while(1);
  }

  Serial.println("OLED initialised. Cycling through all 4 screens.");
}

void loop() {
  // Screen 1 — Shift mode
  Serial.println("Screen 1: Shift mode");
  oled_show_shift_mode(42.5, true);
  delay(3000);

  // Screen 2 — Recovery mode
  Serial.println("Screen 2: Recovery mode");
  oled_show_recovery(74.0, 1.2, 8);
  delay(3000);

  // Screen 3 — READY
  Serial.println("Screen 3: READY");
  oled_show_ready();
  delay(3000);

  // Screen 4 — NOT READY (HRV failed)
  Serial.println("Screen 4: NOT READY");
  oled_show_not_ready(false, true, true, 5);
  delay(3000);
}