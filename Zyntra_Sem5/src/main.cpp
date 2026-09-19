#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "hrv.h"
#include "temperature.h"
#include "reaction_test.h"
#include "oled_display.h"
#include "clearance.h"
#include "mqtt_service.h"

// ─────────────────────────────────────────────────────────────────────────────
// Zyntra — Post-Break Physiological Readiness Clearance System
// WiFi + MQTT Transport Engine for Gym Performance Testing
// ─────────────────────────────────────────────────────────────────────────────

enum ZyntraState {
  STATE_BASELINE,
  STATE_SHIFT,
  STATE_RECOVERY,
  STATE_CLEARANCE,
  STATE_CLEARED,
  STATE_NOT_CLEARED
};

static ZyntraState currentState = STATE_SHIFT;

// Stored baselines
static float hrv_baseline  = 0.0;
static float temp_baseline = 0.0;

// Recovery timing
static uint32_t recovery_start_ms = 0;

// OLED update timing
static uint32_t last_oled_update  = 0;
static uint32_t last_mqtt_vitals  = 0;

// Button debounce
static uint32_t last_button_press = 0;

// Forward declarations
void enter_baseline();
void enter_shift();
void enter_recovery();
void run_clearance();
void enter_cleared(const ClearanceResult& r);
void enter_not_cleared(const ClearanceResult& r);
bool button_pressed();
void smart_delay(uint32_t ms);

// ── Non-blocking delay helper keeping MQTT connection alive ─────────────────
void smart_delay(uint32_t ms) {
  uint32_t start = millis();
  while (millis() - start < ms) {
    mqtt_loop();
    delay(20);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("========================================");
  Serial.println("  ZYNTRA — Starting up (WiFi + MQTT)");
  Serial.println("========================================");

  Wire.begin(PIN_SDA, PIN_SCL);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LED,    OUTPUT);

  // ── Init OLED ────────────────────────────────────────────────────────
  if (!oled_init()) {
    Serial.println("[INIT] OLED failed — halting");
    while (true);
  }
  oled_show_message("ZYNTRA", "Connecting WiFi...");

  // ── Init HRV ─────────────────────────────────────────────────────────
  Serial.println("[INIT] MAX30102...");
  if (!hrv_init()) {
    Serial.println("[INIT] HRV sensor failed — halting");
    oled_show_message("ERROR", "HRV sensor");
    while (true);
  }

  // ── Init Temperature ─────────────────────────────────────────────────
  Serial.println("[INIT] MLX90614...");
  if (!temperature_init()) {
    Serial.println("[INIT] Temp sensor failed — halting");
    oled_show_message("ERROR", "Temp sensor");
    while (true);
  }

  // ── Init Reaction test ────────────────────────────────────────────────
  rt_init();

  // ── Init WiFi + MQTT ─────────────────────────────────────────────────
  Serial.println("[INIT] WiFi & MQTT...");
  if (!mqtt_init()) {
    Serial.println("[INIT] MQTT connect failed — background retries enabled");
  }

  Serial.println("[INIT] All modules ready");
  Serial.println("========================================");

  enter_shift();
}

// ─────────────────────────────────────────────────────────────────────────────
void loop() {
  // Maintain MQTT client loop & process incoming commands
  mqtt_loop();
  uint8_t cmd = mqtt_get_command();

  switch (currentState) {

    // ── SHIFT (Idle / Ready) ───────────────────────────────────────────
    case STATE_SHIFT:
      if (millis() - last_oled_update >= OLED_UPDATE_INTERVAL_MS) {
        last_oled_update = millis();
        oled_show_shift_mode(-1.0, mqtt_is_connected());
      }

      if (cmd == MQTT_CMD_START_BASELINE) {
        Serial.println("[STATE] App requested BASELINE capture");
        enter_baseline();
      } else if (button_pressed() || cmd == MQTT_CMD_START_RECOVERY) {
        Serial.println("[STATE] Recovery test triggered");
        enter_recovery();
      }
      break;

    // ── BASELINE ───────────────────────────────────────────────────────
    case STATE_BASELINE:
      break;

    // ── RECOVERY ───────────────────────────────────────────────────────
    case STATE_RECOVERY: {
      // Feed PPG samples for beat detection during recovery
      hrv_process_sample();

      uint32_t elapsed_ms   = millis() - recovery_start_ms;
      uint32_t remaining_ms = (elapsed_ms < BREAK_DURATION_MS)
                              ? (BREAK_DURATION_MS - elapsed_ms) : 0;
      uint16_t secs_left    = remaining_ms / 1000;
      int      elapsed_min  = elapsed_ms / 60000;

      // OLED refresh every second
      if (millis() - last_oled_update >= OLED_UPDATE_INTERVAL_MS) {
        last_oled_update = millis();
        float rmssd = hrv_compute_rmssd();

        if (rmssd < 0) {
          oled_show_recovery_signal_lost(temperature_get_deviation(), elapsed_min);
        } else {
          float hrv_pct = hrv_get_recovery_percent();
          oled_show_recovery(hrv_pct, temperature_get_deviation(), elapsed_min);
        }
      }

      // MQTT Vitals stream every second
      if (millis() - last_mqtt_vitals >= 1000) {
        last_mqtt_vitals = millis();
        float rmssd       = hrv_compute_rmssd();
        float temp_cur    = temperature_read();
        int16_t rmssd_raw = (rmssd < 0) ? -1 : (int16_t)(rmssd * 10);
        int16_t tbase_raw = (int16_t)(temp_baseline * 10);
        int16_t tcur_raw  = (int16_t)(temp_cur * 10);
        mqtt_notify_vitals(rmssd_raw, tbase_raw, tcur_raw, secs_left);
      }

      // End of recovery window -> run clearance test
      if (remaining_ms == 0) {
        run_clearance();
      }
      break;
    }

    // ── CLEARANCE ──────────────────────────────────────────────────────
    case STATE_CLEARANCE:
      break;

    // ── CLEARED ────────────────────────────────────────────────────────
    case STATE_CLEARED:
      if (button_pressed() || cmd == MQTT_CMD_ACK) {
        Serial.println("[STATE] Verdict acknowledged — returning to SHIFT");
        enter_shift();
      }
      break;

    // ── NOT CLEARED ────────────────────────────────────────────────────
    case STATE_NOT_CLEARED:
      if (button_pressed() || cmd == MQTT_CMD_ACK) {
        Serial.println("[STATE] Verdict acknowledged — returning to SHIFT");
        enter_shift();
      }
      break;
  }
}

// ─────────────────────────────────────────────────────────────────────────────
void enter_baseline() {
  currentState = STATE_BASELINE;
  mqtt_notify_state(MQTT_STATE_BASELINE);
  Serial.println("[STATE] → BASELINE");

  oled_show_message("BASELINE", "Keep finger on");
  smart_delay(2000);

  // ── HRV baseline (blocking) ───────────────────────────────────────────
  Serial.println("[BASELINE] Capturing HRV — hold still...");
  hrv_reset_buffer();
  uint32_t start = millis();

  while (millis() - start < BASELINE_DURATION_MS) {
    hrv_process_sample();
    mqtt_loop(); // Maintain MQTT keep-alives continuously

    int secs_left = (int)((BASELINE_DURATION_MS - (millis() - start)) / 1000);
    if (millis() - last_oled_update >= 1000) {
      last_oled_update = millis();
      char buf[20];
      sprintf(buf, "HRV: %ds left", secs_left);
      oled_show_message("BASELINE", buf);
    }
  }

  hrv_baseline = hrv_compute_rmssd();
  if (hrv_baseline < 0) {
    hrv_baseline = 80.0;
    Serial.println("[BASELINE] HRV insufficient — using default 80ms");
  }
  Serial.print("[BASELINE] HRV baseline: ");
  Serial.print(hrv_baseline);
  Serial.println(" ms");

  // ── Temperature baseline ──────────────────────────────────────────────
  Serial.println("[BASELINE] Capturing temperature...");
  oled_show_message("BASELINE", "Reading temp...");

  // Non-blocking 5 temperature readings with MQTT keep-alives
  float temp_sum = 0.0;
  for (int i = 0; i < BASELINE_TEMP_READINGS; i++) {
    temp_sum += temperature_read();
    smart_delay(500);
  }
  temp_baseline = temp_sum / BASELINE_TEMP_READINGS;
  temperature_set_baseline(temp_baseline);

  Serial.print("[BASELINE] Temp baseline: ");
  Serial.print(temp_baseline);
  Serial.println(" C");

  // ── Reaction time baseline ────────────────────────────────────────────
  Serial.println("[BASELINE] Capturing reaction time...");
  oled_show_message("BASELINE", "Tap when buzzes");
  smart_delay(2000);

  uint16_t rt_baseline = rt_run_test();
  Serial.print("[BASELINE] RT baseline: ");
  Serial.print(rt_baseline);
  Serial.println(" ms");

  // Show baseline summary on OLED
  oled_show_baseline_result(hrv_baseline, temp_baseline, rt_baseline);

  // Publish baseline capture over MQTT to app
  mqtt_notify_baseline(hrv_baseline, temp_baseline, rt_baseline);

  smart_delay(3000);

  // Complete baseline: return to SHIFT idle state
  enter_shift();
}

void enter_shift() {
  currentState = STATE_SHIFT;
  mqtt_notify_state(MQTT_STATE_SHIFT);
  hrv_reset_buffer();
  Serial.println("[STATE] → SHIFT");
  oled_show_message("ZYNTRA READY", "Waiting app command");
}

void enter_recovery() {
  currentState      = STATE_RECOVERY;
  recovery_start_ms = millis();
  last_mqtt_vitals  = 0;
  mqtt_notify_state(MQTT_STATE_RECOVERY);
  hrv_reset_buffer();
  Serial.println("[STATE] → RECOVERY");
  oled_show_message("RECOVERY", "Resting...");
}

void run_clearance() {
  currentState = STATE_CLEARANCE;
  mqtt_notify_state(MQTT_STATE_CLEARANCE);
  Serial.println("[STATE] → CLEARANCE");
  oled_show_message("Break ending...", "Stay relaxed.");

  ClearanceResult result = clearance_run(hrv_baseline, temp_baseline);

  // Notify MQTT broker & app immediately
  mqtt_notify_result(result);

  if (!result.hrv_data_valid) {
    enter_not_cleared(result);
  } else if (result.cleared) {
    enter_cleared(result);
  } else {
    enter_not_cleared(result);
  }
}

void enter_cleared(const ClearanceResult& r) {
  currentState = STATE_CLEARED;
  mqtt_notify_state(MQTT_STATE_CLEARED);
  Serial.println("[STATE] → CLEARED");

  oled_show_clearance_result(
    r.hrv_current, r.temp_deviation, r.rt_median,
    true, r.hrv_pass, r.temp_pass, r.rt_pass, 0
  );

  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_LED, HIGH); delay(200);
    digitalWrite(PIN_LED, LOW);  delay(200);
  }
}

void enter_not_cleared(const ClearanceResult& r) {
  currentState = STATE_NOT_CLEARED;
  mqtt_notify_state(MQTT_STATE_NOT_CLEARED);
  Serial.println("[STATE] → NOT_CLEARED");

  if (!r.hrv_data_valid) {
    oled_show_sensor_error();
  } else {
    oled_show_clearance_result(
      r.hrv_current, r.temp_deviation, r.rt_median,
      false, r.hrv_pass, r.temp_pass, r.rt_pass,
      r.minutes_to_clearance
    );
  }
}

bool button_pressed() {
  if (digitalRead(PIN_BUTTON) == LOW) {
    if (millis() - last_button_press > BUTTON_DEBOUNCE_MS) {
      last_button_press = millis();
      Serial.println("[BTN] Pressed");
      return true;
    }
  }
  return false;
}