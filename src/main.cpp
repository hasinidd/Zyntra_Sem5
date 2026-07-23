#include <Arduino.h>
#include <Wire.h>
#include "hrv.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22);

  Serial.println("=== Zyntra HRV Test ===");

  if (!hrv_init()) {
    Serial.println("MAX30102 failed. Check wiring.");
    while(1);
  }

  Serial.println("Place finger or wrist on MAX30102 sensor.");
  Serial.println("RR intervals and RMSSD will appear as beats are detected.");
}

void loop() {
  // Process PPG sample every loop iteration
  // This must run as fast as possible for accurate beat detection
  hrv_process_sample();

  // Every 10 seconds, print current RMSSD
  static long last_print = 0;
  if (millis() - last_print > 10000) {
    last_print = millis();
    float rmssd = hrv_compute_rmssd();
    if (rmssd > 0) {
      Serial.print("[HRV] Current RMSSD: ");
      Serial.print(rmssd);
      Serial.println(" ms");
    } else {
      Serial.println("[HRV] Waiting for enough beats...");
    }
  }
}