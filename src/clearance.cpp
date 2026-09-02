#include "clearance.h"
#include "hrv.h"
#include "temperature.h"
#include "reaction_test.h"
#include "oled_display.h"

#define HRV_RECOVERY_THRESHOLD   0.90
#define TEMP_RECOVERY_MARGIN_C   0.8
#define RT_THRESHOLD_MS          500

// ── Run clearance protocol ────────────────────────────────────────────────
// This is the AND-gate — all three signals must pass simultaneously
ClearanceResult clearance_run(float hrv_baseline, float temp_baseline) {
  ClearanceResult result;

  Serial.println("[CLR] Running clearance protocol...");

  // ── Signal 1: HRV ────────────────────────────────────────────────────
  // Three possible outcomes: PASS, FAIL, or INSUFFICIENT DATA
  result.hrv_baseline = hrv_baseline;
  result.hrv_current  = hrv_compute_rmssd();

  if (result.hrv_current < 0) {
    // Could not measure HRV at all — sensor contact problem
    // This is a DEVICE issue, not a physiological failure
    result.hrv_pass = false;
    result.hrv_data_valid = false;
    Serial.println("[CLR] HRV: INSUFFICIENT DATA - check sensor contact");
  } else {
    // HRV measured successfully — now check if it passes threshold
    result.hrv_data_valid = true;
    result.hrv_pass = (result.hrv_current >=
                       HRV_RECOVERY_THRESHOLD * hrv_baseline);
    Serial.print("[CLR] HRV: current=");
    Serial.print(result.hrv_current);
    Serial.print("ms baseline=");
    Serial.print(hrv_baseline);
    Serial.print("ms -> ");
    Serial.println(result.hrv_pass ? "PASS" : "FAIL");
  }

  // ── Signal 2: Temperature ────────────────────────────────────────────
  result.temp_deviation = temperature_get_deviation();
  result.temp_pass = (result.temp_deviation <= TEMP_RECOVERY_MARGIN_C);

  Serial.print("[CLR] TEMP: deviation=");
  Serial.print(result.temp_deviation);
  Serial.print("C -> ");
  Serial.println(result.temp_pass ? "PASS" : "FAIL");

  // ── Signal 3: Reaction time ───────────────────────────────────────────
  // Show neutral message only — do NOT reveal RT test is starting
  // Worker must respond naturally without preparation
  // Seeing "prepare to tap" would artificially improve response times
  oled_show_message("Break ending...", "Stay relaxed.");
  delay(5000);

  Serial.println("[CLR] Running RT test now...");
  result.rt_median = rt_run_test();
  result.rt_pass   = rt_is_cleared();

  Serial.print("[CLR] RT: median=");
  Serial.print(result.rt_median);
  Serial.print("ms -> ");
  Serial.println(result.rt_pass ? "PASS" : "FAIL");

  // ── AND-gate decision ─────────────────────────────────────────────────
  // All three must pass simultaneously for clearance
  result.cleared = result.hrv_pass && result.temp_pass && result.rt_pass;

  Serial.print("[CLR] VERDICT: ");
  if (!result.hrv_data_valid) {
    Serial.println("SENSOR ERROR - cannot verify readiness");
  } else {
    Serial.println(result.cleared ? "CLEARED - READY" : "NOT CLEARED - NOT READY");
  }

  // ── Estimate time to clearance if not cleared ─────────────────────────
  if (!result.cleared) {
    result.minutes_to_clearance = clearance_estimate_minutes(
      result.hrv_current,
      hrv_baseline,
      result.temp_deviation
    );
    Serial.print("[CLR] Estimated minutes to clearance: ");
    Serial.println(result.minutes_to_clearance);
  } else {
    result.minutes_to_clearance = 0;
  }

  return result;
}

// ── Estimate minutes to clearance ────────────────────────────────────────
// Simple heuristic based on how far each signal is from its threshold
int clearance_estimate_minutes(float hrv_current, float hrv_baseline,
                                float temp_deviation) {
  int max_estimate = 0;

  // HRV estimate — roughly 2 minutes per 10% of recovery needed
  if (hrv_baseline > 0 && hrv_current > 0) {
    float hrv_recovery_pct = hrv_current / hrv_baseline;
    if (hrv_recovery_pct < HRV_RECOVERY_THRESHOLD) {
      float deficit = HRV_RECOVERY_THRESHOLD - hrv_recovery_pct;
      int hrv_estimate = (int)(deficit * 100 * 2);
      max_estimate = max(max_estimate, hrv_estimate);
    }
  }

  // Temperature estimate — roughly 3 minutes per 0.5C over threshold
  if (temp_deviation > TEMP_RECOVERY_MARGIN_C) {
    float excess = temp_deviation - TEMP_RECOVERY_MARGIN_C;
    int temp_estimate = (int)(excess / 0.5 * 3);
    max_estimate = max(max_estimate, temp_estimate);
  }

  // Minimum estimate of 2 minutes if not cleared
  if (max_estimate < 2) max_estimate = 2;

  return max_estimate;
}