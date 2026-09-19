#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>

// Initialise the SSD1306 OLED display
bool oled_init();

// Screen 1 — Shift mode (passive monitoring during work)
void oled_show_shift_mode(float rmssd, bool mqtt_connected);

// Screen 2 — Recovery mode (during break, live updates)
void oled_show_recovery(float hrv_percent, float temp_deviation, int elapsed_min);

// Screen 2b — Recovery mode with poor signal warning
void oled_show_recovery_signal_lost(float temp_deviation, int elapsed_min);

// NEW: Baseline result — shown after baseline capture completes
// Displays HRV RMSSD, skin temperature, and median reaction time
void oled_show_baseline_result(float hrv_rmssd, float temp_c, uint16_t rt_ms);

// Screen 3 — READY verdict (modified: now shows actual measured values)
void oled_show_ready(float hrv_current, float temp_deviation, uint16_t rt_ms);

// Screen 4 — NOT READY verdict (shows which signal failed + est. minutes)
void oled_show_not_ready(bool hrv_pass, bool temp_pass, bool rt_pass,
                          int minutes_to_clear);

// NEW: Clearance result — shows all 3 values + READY / NOT READY together
// Call this immediately after clearance_run() returns
void oled_show_clearance_result(float hrv_current, float temp_deviation,
                                 uint16_t rt_ms, bool cleared,
                                 bool hrv_pass, bool temp_pass, bool rt_pass,
                                 int minutes_to_clear);

// Screen 5 — Sensor error (device problem, not physiological failure)
void oled_show_sensor_error();

// General two-line message screen
void oled_show_message(const char* line1, const char* line2);

// Countdown screen — testing only
void oled_show_countdown(int seconds_remaining);

#endif