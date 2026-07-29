#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include "hrv.h"

extern MAX30105 particleSensor;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22);

  Serial.println("=== Zyntra HRV Test ===");

  if (!hrv_init()) {
    Serial.println("MAX30102 failed. Check wiring.");
    while(1);
  }

  Serial.println("Place finger firmly on MAX30102.");
  Serial.println("Hold completely still for 60 seconds.");
  Serial.println("RMSSD prints every 30 seconds.");
}

void loop() {
  hrv_process_sample();

  static long last_print = 0;
  if (millis() - last_print > 30000) {
    last_print = millis();
    float rmssd = hrv_compute_rmssd();
    if (rmssd > 0) {
      Serial.print("[HRV] RMSSD: ");
      Serial.print(rmssd);
      Serial.print(" ms  |  BPM: ");
      Serial.println(hrv_get_bpm());
    } else {
      Serial.println("[HRV] Waiting for enough beats...");
    }
  }
}