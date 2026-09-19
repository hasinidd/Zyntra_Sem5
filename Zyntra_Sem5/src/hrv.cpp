#include "hrv.h"
#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

#define MAX_RR_COUNT 200
uint16_t rr_intervals[MAX_RR_COUNT];
int rr_count = 0;

long last_beat_time = 0;
float current_bpm = 0;
static float baseline_rmssd = 0.0;

#define HRV_RECOVERY_PERCENT 0.90

// ── Custom wrist peak detector ────────────────────────────────────────────
// Tuned for wrist PPG pulse range ~300 counts on 141000 baseline
#define PEAK_WINDOW    200    // ~2 seconds at 100Hz
#define PEAK_THRESHOLD 0.55   // 55% of min-max range
#define MIN_RANGE      100    // minimum signal range
#define MIN_RR_MS      500    // minimum 500ms between beats (max 120 BPM)

static long ir_buffer[PEAK_WINDOW];
static int  buf_idx       = 0;
static bool buf_full      = false;
static bool in_peak       = false;
static uint32_t last_peak_time = 0;

bool custom_check_for_beat(long ir_value) {
  ir_buffer[buf_idx] = ir_value;
  buf_idx = (buf_idx + 1) % PEAK_WINDOW;
  if (buf_idx == 0) buf_full = true;
  if (!buf_full) return false;

  long ir_min = ir_buffer[0], ir_max = ir_buffer[0];
  for (int i = 0; i < PEAK_WINDOW; i++) {
    if (ir_buffer[i] < ir_min) ir_min = ir_buffer[i];
    if (ir_buffer[i] > ir_max) ir_max = ir_buffer[i];
  }

  long range = ir_max - ir_min;
  if (range < MIN_RANGE) return false;

  long threshold = ir_min + (long)(range * PEAK_THRESHOLD);

  bool beat = false;
  if (!in_peak && ir_value > threshold) {
    in_peak = true;
    uint32_t now = millis();
    if (now - last_peak_time > MIN_RR_MS) {
      beat = true;
      last_peak_time = now;
    }
  } else if (in_peak && ir_value < threshold) {
    in_peak = false;
  }
  return beat;
}

bool hrv_init() {
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("[HRV] ERROR: MAX30102 not found");
    return false;
  }

  particleSensor.setup(
    50,    // LED brightness
    4,     // Sample average
    2,     // LED mode: Red + IR
    100,   // Sample rate
    411,   // Pulse width
    4096   // ADC range
  );

  memset(ir_buffer, 0, sizeof(ir_buffer));
  buf_idx = 0; buf_full = false;
  in_peak = false; last_peak_time = 0;

  Serial.println("[HRV] MAX30102 initialised successfully");
  return true;
}

void hrv_process_sample() {
  long ir_value = particleSensor.getIR();

  if (ir_value < 50000) return;

  if (custom_check_for_beat(ir_value)) {
    long now = millis();
    uint16_t rr = (uint16_t)(now - last_beat_time);
    last_beat_time = now;

    if (rr > 450 && rr < 1500) {
      // Loose artifact rejection for wrist — 90%
      if (rr_count > 0) {
        float prev_rr = rr_intervals[rr_count - 1];
        float diff_pct = abs(rr - prev_rr) / prev_rr;
        if (diff_pct > 0.90) {
          Serial.println("[HRV] Artifact — skipping");
          return;
        }
      }

      if (rr_count < MAX_RR_COUNT) {
        rr_intervals[rr_count++] = rr;
      } else {
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

float hrv_compute_rmssd() {
  if (rr_count < 5) {
    return -1.0;
  }

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
  return sqrt(sum_sq_diff / valid_pairs);
}

void hrv_capture_baseline() {
  Serial.println("[HRV] Capturing HRV baseline...");
  rr_count = 0;
  memset(rr_intervals, 0, sizeof(rr_intervals));

  long start_time = millis();
  while (millis() - start_time < 180000) {
    hrv_process_sample();
    long elapsed = (millis() - start_time) / 1000;
    if (elapsed % 30 == 0 && elapsed > 0) {
      Serial.print("[HRV] Baseline: ");
      Serial.print(elapsed);
      Serial.println("s / 180s");
    }
  }

  baseline_rmssd = hrv_compute_rmssd();
  Serial.print("[HRV] Baseline RMSSD: ");
  Serial.print(baseline_rmssd);
  Serial.println(" ms");
}

float hrv_get_baseline()         { return baseline_rmssd; }
float hrv_get_bpm()              { return current_bpm; }

bool hrv_is_cleared() {
  float current = hrv_compute_rmssd();
  if (current < 0) return false;
  return (current >= HRV_RECOVERY_PERCENT * baseline_rmssd);
}

float hrv_get_recovery_percent() {
  if (baseline_rmssd <= 0) return 0.0;
  float current = hrv_compute_rmssd();
  if (current < 0) return 0.0;
  return (current / baseline_rmssd) * 100.0;
}

void hrv_reset_buffer() {
  rr_count = 0;
  memset(rr_intervals, 0, sizeof(rr_intervals));
  last_beat_time = millis();
  memset(ir_buffer, 0, sizeof(ir_buffer));
  buf_idx = 0; buf_full = false; in_peak = false;
  Serial.println("[HRV] RR buffer cleared");
}