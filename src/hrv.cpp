#include "hrv.h"
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

// Sensor object
MAX30105 particleSensor;

// ── RR Interval Storage ───────────────────────────────────────────────────
#define MAX_RR_COUNT 200
uint16_t rr_intervals[MAX_RR_COUNT];
int rr_count = 0;

// Beat detection variables
long last_beat_time = 0;
float current_bpm = 0;

// Baseline storage
static float baseline_rmssd = 0.0;

// RMSSD recovery threshold
#define HRV_RECOVERY_PERCENT 0.90

// ── Initialise sensor ─────────────────────────────────────────────────────
bool hrv_init() {
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("[HRV] ERROR: MAX30102 not found");
    return false;
  }

  // Brightness 50 — calibrated for this specific module
  // Gives IR values around 100000-118000 which is the usable range
  // Lower than this = signal too weak for beat detection
  // Higher than this = signal saturates at 262143
  particleSensor.setup(
    50,    // LED brightness — calibrated for this module
    4,     // Sample average — smooths the signal
    2,     // LED mode: Red + IR
    100,   // Sample rate — stable at 100 samples/second
    411,   // Pulse width — highest resolution
    4096   // ADC range — full scale
  );

  Serial.println("[HRV] MAX30102 initialised successfully");
  return true;
}

// ── Process one PPG sample ────────────────────────────────────────────────
// Call this every loop iteration for accurate beat detection
void hrv_process_sample() {
  long ir_value = particleSensor.getIR();

  // Signal quality check — below 50000 means no skin contact
  if (ir_value < 50000) {
    return;
  }

  if (checkForBeat(ir_value) == true) {
    long now = millis();
    uint16_t rr = (uint16_t)(now - last_beat_time);
    last_beat_time = now;

    // Strict physiological limits
    // 500ms to 1200ms = 50 to 120 BPM (normal resting adult range)
    // This rejects:
    // - Values below 500ms (above 120 BPM) = double peak detections
    // - Values above 1200ms (below 50 BPM) = missed beats
    if (rr > 450 && rr < 1300) {

      // Artifact rejection — 25% threshold
      // Rejects intervals that differ too much from the previous one
      // Catches movement artifacts and ectopic beats
      if (rr_count > 0) {
        float prev_rr = rr_intervals[rr_count - 1];
        float diff_percent = abs(rr - prev_rr) / prev_rr;
        if (diff_percent > 0.35) {
          Serial.println("[HRV] Artifact — skipping");
          return;
        }
      }

      // Store valid RR interval
      if (rr_count < MAX_RR_COUNT) {
        rr_intervals[rr_count++] = rr;
      } else {
        // Buffer full — sliding window, drop oldest
        memmove(rr_intervals, rr_intervals + 1,
                (MAX_RR_COUNT - 1) * sizeof(uint16_t));
        rr_intervals[MAX_RR_COUNT - 1] = rr;
      }

      current_bpm = 60000.0 / rr;

      Serial.print("[HRV] Beat detected. RR=");
      Serial.print(rr);
      Serial.print("ms  BPM=");
      Serial.println(current_bpm);
    }
  }
}

// ── Compute RMSSD ─────────────────────────────────────────────────────────
// RMSSD = Root Mean Square of Successive Differences
// Higher RMSSD = more HRV = parasympathetic active = recovered
float hrv_compute_rmssd() {
  // Need at least 10 valid RR intervals for meaningful RMSSD
  if (rr_count < 10) {
    Serial.println("[HRV] Not enough RR intervals yet for RMSSD");
    return -1.0;
  }

  // Use last 140 intervals (~2 minutes at 70 BPM)
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
// Collect RR intervals for 3 minutes at shift start
// Worker must be seated and resting — no movement
void hrv_capture_baseline() {
  Serial.println("[HRV] Capturing HRV baseline — sit still for 3 minutes...");

  // Clear existing data
  rr_count = 0;
  memset(rr_intervals, 0, sizeof(rr_intervals));

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

// ── Get stored baseline ───────────────────────────────────────────────────
float hrv_get_baseline() {
  return baseline_rmssd;
}

// ── Check if HRV has recovered ────────────────────────────────────────────
// Returns true when current RMSSD >= 90% of morning baseline
bool hrv_is_cleared() {
  float current = hrv_compute_rmssd();
  if (current < 0) return false;
  return (current >= HRV_RECOVERY_PERCENT * baseline_rmssd);
}

// ── Get HRV recovery as percentage of baseline ────────────────────────────
float hrv_get_recovery_percent() {
  if (baseline_rmssd <= 0) return 0.0;
  float current = hrv_compute_rmssd();
  if (current < 0) return 0.0;
  return (current / baseline_rmssd) * 100.0;
}

// ── Get current heart rate in BPM ────────────────────────────────────────
float hrv_get_bpm() {
  return current_bpm;
}