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

// Screen 3 — READY verdict
// Shown when all three signals pass clearance
void oled_show_ready();

// Screen 4 — NOT READY verdict
// Shows which signal failed and estimated minutes to clearance
void oled_show_not_ready(bool hrv_pass, bool temp_pass, bool rt_pass, int minutes_to_clear);

// General message screen — used for baseline capture prompts
void oled_show_message(const char* line1, const char* line2);

#endif