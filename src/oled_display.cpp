#include "oled_display.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// Your OLED is 128x32
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
void oled_show_shift_mode(float rmssd, bool ble_connected) {
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
    display.print("Reading...");
  }
  display.setCursor(0, 24);
  display.print(ble_connected ? "BLE: Connected" : "BLE: Searching");
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

// ── Screen 2b: Recovery Mode with poor signal warning ────────────────────
// Shown when HRV sensor loses contact during recovery
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

// ── Screen 3: READY ───────────────────────────────────────────────────────
void oled_show_ready() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(16, 2);
  display.println("READY");
  display.setTextSize(1);
  display.setCursor(10, 22);
  display.print("HRV  TEMP  RT: PASS");
  display.display();
}

// ── Screen 4: NOT READY ───────────────────────────────────────────────────
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

// ── Screen 5: Sensor Error ────────────────────────────────────────────────
// Shown when HRV could not be measured at all
// This is a DEVICE problem not a physiological failure
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
// Used during testing only — not in real wristband system
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