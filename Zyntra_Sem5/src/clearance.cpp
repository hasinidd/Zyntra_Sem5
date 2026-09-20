#include "clearance.h"
#include "hrv.h"
#include "temperature.h"
#include "reaction_test.h"
#include "oled_display.h"

#define HRV_RECOVERY_THRESHOLD   0.90
#define DEFAULT_TEMP_MARGIN_C    0.8
#define DEFAULT_RT_THRESHOLD_MS  500

// ── Age-dependent threshold helper functions ──────────────────────────────
uint16_t clearance_get_rt_threshold_for_age(uint8_t age) {
  if (age == 0)   return DEFAULT_RT_THRESHOLD_MS; // Fallback default
  if (age <= 25)  return 450; // Age 18–25: Fatigued limit ~450ms (Blomkvist et al. 2017)
  if (age <= 35)  return 480; // Age 26–35: Fatigued limit ~480ms
  if (age <= 45)  return 500; // Age 36–45: Fatigued limit ~500ms
  if (age <= 55)  return 530; // Age 46–55: Fatigued limit ~530ms
  if (age <= 65)  return 580; // Age 56–65: Fatigued limit ~580ms
  return 650;                 // Age 65+:   Fatigued limit ~650ms
}

float clearance_get_temp_margin_for_age(uint8_t age) {
  if (age > 45) return 0.6; // Reduced vasodilation capacity in older adults (Lifestack 2025)
  return DEFAULT_TEMP_MARGIN_C;
}

// ── Run clearance protocol ────────────────────────────────────────────────
// This is the AND-gate — all three signals must pass simultaneously
ClearanceResult clearance_run(float hrv_baseline, float temp_baseline, uint8_t user_age) {
  ClearanceResult result;
  uint16_t rt_threshold = clearance_get_rt_threshold_for_age(user_age);
  float temp_margin     = clearance_get_temp_margin_for_age(user_age);

  Serial.println("[CLR] Running clearance protocol with age-dependent thresholds...");
  Serial.print("[CLR] Participant Age: "); Serial.println(user_age);
  Serial.print("[CLR] Dynamic RT Pass Threshold: < "); Serial.print(rt_threshold); Serial.println(" ms");
  Serial.print("[CLR] Dynamic Temp Margin: <= "); Serial.print(temp_margin); Serial.println(" C");

  // ── Signal 1: HRV ────────────────────────────────────────────────────
  result.hrv_baseline = hrv_baseline;
  result.hrv_current  = hrv_compute_rmssd();

  if (result.hrv_current < 0) {
    result.hrv_pass = false;
    result.hrv_data_valid = false;
    Serial.println("[CLR] HRV: INSUFFICIENT DATA - check sensor contact");
  } else {
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
  float temp_current = temperature_read();
  result.temp_deviation = abs(temp_current - temp_baseline);
  result.temp_pass = (result.temp_deviation <= temp_margin);

  Serial.print("[CLR] TEMP: deviation=");
  Serial.print(result.temp_deviation);
  Serial.print("C (margin: "); Serial.print(temp_margin);
  Serial.print("C) -> ");
  Serial.println(result.temp_pass ? "PASS" : "FAIL");

  // ── Signal 3: Reaction time ───────────────────────────────────────────
  oled_show_message("Break ending...", "Stay relaxed.");
  delay(5000);

  Serial.println("[CLR] Running RT test now...");
  result.rt_median = rt_run_test();
  result.rt_pass   = (result.rt_median < rt_threshold);

  Serial.print("[CLR] RT: median=");
  Serial.print(result.rt_median);
  Serial.print("ms (threshold: "); Serial.print(rt_threshold);
  Serial.print("ms) -> ");
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
      result.temp_deviation,
      user_age
    );
    Serial.print("[CLR] Estimated minutes to clearance: ");
    Serial.println(result.minutes_to_clearance);
  } else {
    result.minutes_to_clearance = 0;
  }

  return result;
}

// ── Estimate minutes to clearance ────────────────────────────────────────
// Research-backed estimation (Stöggl & Sperlich 2019, PMC6981425)
// Moderate post-exercise recovery takes 10–30 min maximum
int clearance_estimate_minutes(float hrv_current, float hrv_baseline,
                                float temp_deviation, uint8_t user_age) {
  int max_estimate = 0;

  // 1. HRV Estimate — RMSSD returns to baseline within 15–30 min for moderate exercise
  if (hrv_baseline > 0 && hrv_current > 0) {
    float hrv_recovery_pct = hrv_current / hrv_baseline;
    if (hrv_recovery_pct < HRV_RECOVERY_THRESHOLD) {
      float deficit = HRV_RECOVERY_THRESHOLD - hrv_recovery_pct;
      // Each 10% deficit = ~3 minutes (capped at 30 min for moderate exercise)
      int hrv_estimate = (int)(deficit * 100 * 3);
      if (hrv_estimate > 30) hrv_estimate = 30;
      max_estimate = max(max_estimate, hrv_estimate);
    }
  }

  // 2. Temperature Estimate — skin temp recovers within 10–20 min
  float temp_margin = clearance_get_temp_margin_for_age(user_age);
  if (temp_deviation > temp_margin) {
    float excess = temp_deviation - temp_margin;
    // Each 0.5°C excess = ~5 minutes (capped at 20 min)
    int temp_estimate = (int)(excess / 0.5 * 5);
    if (temp_estimate > 20) temp_estimate = 20;
    max_estimate = max(max_estimate, temp_estimate);
  }

  // 3. Realistic bounds: 5 min minimum rest if not cleared, 30 min maximum ceiling
  if (max_estimate < 5) max_estimate = 5;
  if (max_estimate > 30) max_estimate = 30;

  return max_estimate;
}