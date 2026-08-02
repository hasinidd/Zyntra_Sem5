#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include "hrv.h"
#include "temperature.h"
#include "reaction_test.h"
#include "clearance.h"
#include "oled_display.h"

extern MAX30105 particleSensor;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22);

  Serial.println("=== Zyntra Clearance Algorithm Test ===");

  // Initialise all modules
  if (!hrv_init())         { Serial.println("HRV init failed");  while(1); }
  if (!temperature_init()) { Serial.println("Temp init failed"); while(1); }
  if (!oled_init())        { Serial.println("OLED init failed"); while(1); }
  rt_init();

  // Set temperature baseline to current reading for testing
  // In real system this comes from temperature_capture_baseline()
  // captured at shift start when worker first puts on the device
  temperature_set_baseline(temperature_read());

  Serial.println("All modules initialised.");
  Serial.println("Place finger on MAX30102 and hold still for 90 seconds.");

  // Live countdown while collecting HRV data
  // Updates every 1 second for smooth visible countdown
  long countdown_start = millis();
  long countdown_duration = 90000;  // 90 seconds
  long last_oled_update = 0;

  while (millis() - countdown_start < countdown_duration) {
    // Keep processing PPG samples during countdown
    hrv_process_sample();

    // Update OLED every 1 second
    if (millis() - last_oled_update > 1000) {
      last_oled_update = millis();
      int seconds_left = (int)((countdown_duration -
                         (millis() - countdown_start)) / 1000);
      oled_show_countdown(seconds_left);
      Serial.print("[MAIN] Countdown: ");
      Serial.print(seconds_left);
      Serial.println("s remaining — keep finger on sensor.");
    }
  }

  Serial.println("Data collection complete. Starting clearance protocol.");
}

void loop() {
  // Continuously process PPG samples every loop iteration
  // This must run as fast as possible for accurate beat detection
  hrv_process_sample();

  // Run clearance test every 60 seconds
  static long last_test = 0;
  if (millis() - last_test > 60000) {
    last_test = millis();

    Serial.println("\n=== RUNNING CLEARANCE PROTOCOL ===");

    // Simulated HRV baseline for testing
    // In real system this comes from morning baseline capture
    float test_hrv_baseline = 80.0;

    ClearanceResult result = clearance_run(test_hrv_baseline, 0.0);

    // Three possible display outcomes
    if (!result.hrv_data_valid) {
      // Device problem — HRV could not be measured at all
      // Worker should adjust the wristband and retry
      Serial.println("[MAIN] Showing SENSOR ERROR screen");
      oled_show_sensor_error();

    } else if (result.cleared) {
      // All three signals passed — worker is cleared
      Serial.println("[MAIN] Showing READY screen");
      oled_show_ready();

    } else {
      // HRV was measured but one or more signals did not recover
      Serial.println("[MAIN] Showing NOT READY screen");
      oled_show_not_ready(
        result.hrv_pass,
        result.temp_pass,
        result.rt_pass,
        result.minutes_to_clearance
      );
    }

    Serial.println("=== CLEARANCE PROTOCOL COMPLETE ===\n");
  }
}