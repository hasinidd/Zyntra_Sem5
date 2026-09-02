#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>

// Initialise the SSD1306 OLED display
// Returns true if found, false if not
bool oled_init();

// Screen 1 — Shift mode (passive monitoring during work)
// Shows current HRV level and BLE connection status
void oled_show_shift_mode(float rmssd, bool ble_connected);

// Screen 2 — Recovery mode (during break)
// Shows three recovery progress values updating live
void oled_show_recovery(float hrv_percent, float temp_deviation, int elapsed_min);

// Screen 2b — Recovery mode with poor signal warning
// Shown when HRV sensor loses skin contact during recovery
void oled_show_recovery_signal_lost(float temp_deviation, int elapsed_min);

// Screen 3 — READY verdict
// Shown when all three signals pass clearance
void oled_show_ready();

// Screen 4 — NOT READY verdict
// Shows which signal failed and estimated minutes to clearance
void oled_show_not_ready(bool hrv_pass, bool temp_pass, bool rt_pass,
                          int minutes_to_clear);

// Screen 5 — Sensor error
// Shown when HRV could not be measured at all
// This is a DEVICE problem not a physiological failure
void oled_show_sensor_error();

// General message screen — two lines of text
void oled_show_message(const char* line1, const char* line2);

// Countdown screen — used during testing only
// In real wristband no countdown needed as device is worn continuously
void oled_show_countdown(int seconds_remaining);

#endif