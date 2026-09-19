#include "temperature.h"
#include <Adafruit_MLX90614.h>

// Create sensor object
Adafruit_MLX90614 mlx;

// Store the baseline temperature captured at shift start
static float baseline_temp = 0.0;

// How far current temp must be from baseline to be cleared (degrees C)
#define TEMP_RECOVERY_MARGIN 0.8

// ── Initialise the sensor ─────────────────────────────────────────────────
bool temperature_init() {
  if (!mlx.begin()) {
    Serial.println("[TEMP] ERROR: MLX90614 not found on I2C bus");
    return false;
  }
  delay(1000); // Allow sensor to fully stabilise before first reading
  Serial.println("[TEMP] MLX90614 initialised successfully");
  Serial.print("[TEMP] Ambient temp: ");
  Serial.print(mlx.readAmbientTempC());
  Serial.println(" C");
  Serial.print("[TEMP] Object temp: ");
  Serial.print(mlx.readObjectTempC());
  Serial.println(" C");
  return true;
}

// ── Capture baseline at shift start ───────────────────────────────────────
// Takes 5 readings spaced 30 seconds apart and averages them
// Worker must be sitting still and resting during this time
void temperature_capture_baseline() {
  Serial.println("[TEMP] Capturing baseline — sit still for 2.5 minutes...");
  float total = 0.0;
  int readings = 5;

  for (int i = 0; i < readings; i++) {
    float reading = mlx.readObjectTempC();
    total += reading;
    Serial.print("[TEMP] Reading ");
    Serial.print(i + 1);
    Serial.print("/5: ");
    Serial.print(reading);
    Serial.println(" C");

    if (i < readings - 1) {
      delay(30000); // Wait 30 seconds between readings
    }
  }

  baseline_temp = total / readings;
  Serial.print("[TEMP] Baseline captured: ");
  Serial.print(baseline_temp);
  Serial.println(" C");
}

// ── Manually set baseline ─────────────────────────────────────────────────
// Used for testing without full baseline capture
void temperature_set_baseline(float value) {
  baseline_temp = value;
  Serial.print("[TEMP] Baseline manually set to: ");
  Serial.print(value);
  Serial.println(" C");
}

// ── Read current wrist temperature ────────────────────────────────────────
float temperature_read() {
  return mlx.readObjectTempC();
}

// ── Get stored baseline ───────────────────────────────────────────────────
float temperature_get_baseline() {
  return baseline_temp;
}

// ── Check if temperature has recovered ───────────────────────────────────
bool temperature_is_cleared() {
  float current = mlx.readObjectTempC();
  float deviation = abs(current - baseline_temp);
  return (deviation <= TEMP_RECOVERY_MARGIN);
}

// ── Get deviation from baseline ───────────────────────────────────────────
float temperature_get_deviation() {
  float current = mlx.readObjectTempC();
  return abs(current - baseline_temp);
}