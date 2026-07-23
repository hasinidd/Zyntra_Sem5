#include "hrv.h"
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

// Sensor object
MAX30105 particleSensor;

// ── RR Interval Storage ───────────────────────────────────────────────────
// We store the last 200 RR intervals (enough for ~3 minutes at 70bpm)
#define MAX_RR_COUNT 200
uint16_t rr_intervals[MAX_RR_COUNT];  // in milliseconds
int rr_count = 0;

// Beat detection variables
long last_beat_time = 0;
float current_bpm = 0;

// Baseline storage
static float baseline_rmssd = 0.0;

// RMSSD recovery threshold
#define HRV_RECOVERY_PERCENT 0.90  // Must reach 90% of baseline

// ── Initialise sensor ─────────────────────────────────────────────────────
bool hrv_init() {
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("[HRV] ERROR: MAX30102 not found");
    return false;
  }

  // Configure for HRV-optimised mode
  // These settings balance accuracy with power consumption
  particleSensor.setup(
    60,    // LED brightness (0-255). 60 = ~12mA — good for wrist contact
    4,     // Sample average: 4 samples averaged = smoother signal
    2,     // LED mode: 2 = Red + IR (we use IR for HRV)
    400,   // Sample rate: 400 samples/second
    411,   // Pulse width: 411 microseconds = highest resolution
    4096   // ADC range: 4096 nA full scale
  );

  Serial.println("[HRV] MAX30102 initialised successfully");
  return true;
}

// ── Process one PPG sample ────────────────────────────────────────────────
// Call this every loop iteration
// checkForBeat() analyses the IR value and returns true when a beat is detected
void hrv_process_sample() {
  long ir_value = particleSensor.getIR();

  // Check signal quality — if IR value is too low, no finger/wrist contact
  if (ir_value < 50000) {
    // No contact — do not process
    return;
  }

  // Check for heartbeat using SparkFun library algorithm
  if (checkForBeat(ir_value) == true) {
    long now = millis();
    uint16_t rr = (uint16_t)(now - last_beat_time);
    last_beat_time = now;

    // Validate RR interval is physiologically plausible
    // Normal human HR range: 30-200 bpm = RR 300ms to 2000ms
    if (rr > 300 && rr < 2000) {

      // Artifact rejection: reject if >20% different from previous interval
      // This filters out movement artifacts and ectopic beats
      if (rr_count > 0) {
        float prev_rr = rr_intervals[rr_count - 1];
        float diff_percent = abs(rr - prev_rr) / prev_rr;
        if (diff_percent > 0.20) {
          // Too different from previous — likely artifact, skip it
          Serial.println("[HRV] Artifact detected — skipping interval");
          return;
        }
      }

      // Store the RR interval
      if (rr_count < MAX_RR_COUNT) {
        rr_intervals[rr_count++] = rr;
      } else {
        // Buffer full — shift left and add new at end (sliding window)
        memmove(rr_intervals, rr_intervals + 1,
                (MAX_RR_COUNT - 1) * sizeof(uint16_t));
        rr_intervals[MAX_RR_COUNT - 1] = rr;
      }

      // Compute BPM for display
      current_bpm = 60000.0 / rr;

      Serial.print("[HRV] Beat detected. RR=");
      Serial.print(rr);
      Serial.print("ms  BPM=");
      Serial.println(current_bpm);
    }
  }
}

// ── Compute RMSSD ─────────────────────────────────────────────────────────
// RMSSD = Root Mean Square of Successive Differences between RR intervals
// Higher RMSSD = more HRV = parasympathetic system active = recovered
float hrv_compute_rmssd() {
  // Need at least 10 RR intervals for meaningful RMSSD
  if (rr_count < 10) {
    Serial.println("[HRV] Not enough RR intervals yet for RMSSD");
    return -1.0;
  }

  // Use last 140 intervals (~2 minutes at 70bpm)
  int n = min(rr_count, 140);
  int start = rr_count - n;

  float sum_sq_diff = 0.0;
  int valid_pairs = 0;

  for (int i = start + 1; i < rr_count; i++) {
    float diff = (float)rr_intervals[i] - (float)rr_intervals[i - 1];
    sum_sq_diff += diff * diff;
    valid_pairs++;
  }

  if (valid_pairs == 0) return -1.0;

  float rmssd = sqrt(sum_sq_diff / valid_pairs);
  return rmssd;
}

// ── Capture baseline ──────────────────────────────────────────────────────
// Collect RR intervals for 3 minutes and compute baseline RMSSD
// Worker must be sitting still and resting during this time
void hrv_capture_baseline() {
  Serial.println("[HRV] Capturing HRV baseline — sit still for 3 minutes...");

  // Clear existing RR intervals
  rr_count = 0;
  memset(rr_intervals, 0, sizeof(rr_intervals));

  // Collect samples for 3 minutes (180,000 ms)
  long start_time = millis();
  long duration = 180000; // 3 minutes

  while (millis() - start_time < duration) {
    hrv_process_sample();

    // Print progress every 30 seconds
    long elapsed = (millis() - start_time) / 1000;
    if (elapsed % 30 == 0 && elapsed > 0) {
      Serial.print("[HRV] Baseline progress: ");
      Serial.print(elapsed);
      Serial.println(" / 180 seconds");
    }
  }

  baseline_rmssd = hrv_compute_rmssd();
  Serial.print("[HRV] Baseline RMSSD: ");
  Serial.print(baseline_rmssd);
  Serial.println(" ms");
}

// ── Get baseline ──────────────────────────────────────────────────────────
float hrv_get_baseline() {
  return baseline_rmssd;
}

// ── Check if HRV has recovered ────────────────────────────────────────────
bool hrv_is_cleared() {
  float current = hrv_compute_rmssd();
  if (current < 0) return false;
  return (current >= HRV_RECOVERY_PERCENT * baseline_rmssd);
}

// ── Get recovery percentage ───────────────────────────────────────────────
float hrv_get_recovery_percent() {
  if (baseline_rmssd <= 0) return 0.0;
  float current = hrv_compute_rmssd();
  if (current < 0) return 0.0;
  return (current / baseline_rmssd) * 100.0;
}

// ── Get current BPM ───────────────────────────────────────────────────────
float hrv_get_bpm() {
  return current_bpm;
}