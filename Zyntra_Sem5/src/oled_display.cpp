#include "oled_display.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  32
#define OLED_RESET     -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ── Initialise display ────────────────────────────────────────────────────
bool oled_init() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("[OLED] ERROR: SSD1306 not found");
    return false;
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.display();
  Serial.println("[OLED] SSD1306 initialised successfully");
  return true;
}

// ── Screen 1: Shift Mode ──────────────────────────────────────────────────
void oled_show_shift_mode(float rmssd, bool mqtt_connected) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ZYNTRA - SHIFT MODE");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
  display.setCursor(0, 13);
  display.print("HRV: ");
  if (rmssd > 0) {
    display.print(rmssd, 1);
    display.print(" ms");
  } else {
    display.print("Idle");
  }
  display.setCursor(0, 24);
  display.print(mqtt_connected ? "MQTT: Connected" : "MQTT: Offline");
  display.display();
}

// ── Screen 2: Recovery Mode ───────────────────────────────────────────────
void oled_show_recovery(float hrv_percent, float temp_deviation, int elapsed_min) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("RECOVERING ");
  display.print(elapsed_min);
  display.print("min");
  display.setCursor(0, 10);
  display.print("HRV:");
  int hrv_bar = (int)(hrv_percent * 0.50);
  hrv_bar = min(hrv_bar, 50);
  display.fillRect(28, 10, hrv_bar, 6, SSD1306_WHITE);
  display.drawRect(28, 10, 50, 6, SSD1306_WHITE);
  display.setCursor(0, 20);
  display.print("TEMP:");
  display.print(temp_deviation, 1);
  display.print("C off");
  display.display();
}

// ── Screen 2b: Recovery with signal lost ─────────────────────────────────
void oled_show_recovery_signal_lost(float temp_deviation, int elapsed_min) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("RECOVERING ");
  display.print(elapsed_min);
  display.print("min");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
  display.setCursor(0, 13);
  display.print("HRV: SIGNAL LOST");
  display.setCursor(0, 23);
  display.print("Adjust wristband");
  display.display();
}

// ── NEW: Baseline result screen ───────────────────────────────────────────
// Shown after baseline capture completes — displays all 3 captured values
// Layout (128x32):
//   BASELINE CAPTURED
//   ─────────────────
//   H:80.3ms T:33.2C
//   RT: 392ms
void oled_show_baseline_result(float hrv_rmssd, float temp_c, uint16_t rt_ms) {
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("BASELINE CAPTURED");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // Row 2: HRV and Temp side by side
  display.setCursor(0, 13);
  display.print("H:");
  display.print(hrv_rmssd, 1);
  display.print("ms");

  display.setCursor(68, 13);
  display.print("T:");
  display.print(temp_c, 1);
  display.print("C");

  // Row 3: Reaction time
  display.setCursor(0, 23);
  display.print("RT:");
  display.print(rt_ms);
  display.print("ms");

  display.display();

  Serial.print("[OLED] Baseline shown — HRV:");
  Serial.print(hrv_rmssd, 1);
  Serial.print("ms  TEMP:");
  Serial.print(temp_c, 1);
  Serial.print("C  RT:");
  Serial.print(rt_ms);
  Serial.println("ms");
}

// ── Screen 3: READY ───────────────────────────────────────────────────────
// Modified — now shows actual measured values alongside PASS verdict
// Layout:
//   ** READY **
//   ─────────────────
//   H:88ms T:0.2C RT:379
void oled_show_ready(float hrv_current, float temp_deviation, uint16_t rt_ms) {
  display.clearDisplay();

  // Large READY text
  display.setTextSize(2);
  display.setCursor(10, 0);
  display.print("** READY");

  display.setTextSize(1);
  display.drawLine(0, 17, 128, 17, SSD1306_WHITE);

  // Values on bottom row
  display.setCursor(0, 22);
  display.print("H:");
  display.print(hrv_current, 0);
  display.print(" T:");
  display.print(temp_deviation, 1);
  display.print(" R:");
  display.print(rt_ms);

  display.display();
}

// ── Screen 4: NOT READY ───────────────────────────────────────────────────
// Modified — shows actual values AND pass/fail for each signal
// Layout:
//   NOT READY
//   ─────────────────
//   H:65ms(FAIL) T:0.3
//   RT:480ms  +5min
void oled_show_not_ready(bool hrv_pass, bool temp_pass, bool rt_pass,
                          int minutes_to_clear) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("NOT READY");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
  display.setCursor(0, 12);
  display.print("HRV:");
  display.print(hrv_pass ? "OK" : "FAIL");
  display.setCursor(48, 12);
  display.print("TMP:");
  display.print(temp_pass ? "OK" : "FAIL");
  display.setCursor(96, 12);
  display.print("RT:");
  display.print(rt_pass ? "OK" : "NO");
  display.setCursor(0, 23);
  display.print("Est. ready: ");
  display.print(minutes_to_clear);
  display.print(" min");
  display.display();
}

// ── NEW: Clearance result screen ──────────────────────────────────────────
// Shows all 3 measured values together with READY / NOT READY verdict
// Called immediately after clearance_run() completes
// Layout (READY case):
//   CLEARED - READY
//   ─────────────────
//   H:88ms T:0.2C
//   RT:379ms  ALL PASS
//
// Layout (NOT READY case):
//   NOT READY
//   ─────────────────
//   H:65ms T:0.9C
//   RT:480ms  +5min
void oled_show_clearance_result(float hrv_current, float temp_deviation,
                                 uint16_t rt_ms, bool cleared,
                                 bool hrv_pass, bool temp_pass, bool rt_pass,
                                 int minutes_to_clear) {
  display.clearDisplay();
  display.setTextSize(1);

  // Row 1: Verdict
  display.setCursor(0, 0);
  if (cleared) {
    display.print("CLEARED - READY");
  } else {
    display.print("NOT READY");
  }
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // Row 2: HRV and Temp values
  display.setCursor(0, 12);
  display.print("H:");
  if (hrv_current > 0) {
    display.print(hrv_current, 0);
    display.print("ms");
  } else {
    display.print("ERR");
  }

  display.setCursor(55, 12);
  display.print("T:");
  display.print(temp_deviation, 1);
  display.print("C");

  // Row 3: RT and summary
  display.setCursor(0, 22);
  display.print("RT:");
  display.print(rt_ms);
  display.print("ms");

  if (cleared) {
    display.setCursor(68, 22);
    display.print("ALL PASS");
  } else {
    display.setCursor(68, 22);
    display.print("+");
    display.print(minutes_to_clear);
    display.print("min");
  }

  display.display();

  Serial.print("[OLED] Clearance result shown — ");
  Serial.println(cleared ? "READY" : "NOT READY");
}

// ── Screen 5: Sensor Error ────────────────────────────────────────────────
void oled_show_sensor_error() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SENSOR ERROR");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
  display.setCursor(0, 13);
  display.print("Cannot read HRV");
  display.setCursor(0, 23);
  display.print("Adjust and retry");
  display.display();
}

// ── General message screen ────────────────────────────────────────────────
void oled_show_message(const char* line1, const char* line2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 8);
  display.println(line1);
  display.setCursor(0, 20);
  display.println(line2);
  display.display();
}

// ── Countdown screen ──────────────────────────────────────────────────────
void oled_show_countdown(int seconds_remaining) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Hold finger on sensor");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
  display.setCursor(0, 13);
  display.print("HRV collecting...");
  display.setCursor(0, 23);
  display.print("Starting in ");
  display.print(seconds_remaining);
  display.print("s");
  display.display();
}