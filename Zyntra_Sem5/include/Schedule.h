#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <Arduino.h>

// ── BREAK TYPES ────────────────────────────────────────────────────────────
enum BreakType {
  BREAK_SHORT,   // 15-minute break
  BREAK_LONG     // 1-hour break
};

// ── BREAK WINDOW ───────────────────────────────────────────────────────────
// Represents one scheduled break for the day.
// start_ms and end_ms are elapsed milliseconds since shift start (millis()
// based for testing). In production these will be wall-clock times from
// the ESP32's NTP-synced RTC, converted to elapsed-since-shift-start the
// same way, so the rest of the logic never has to change.
struct BreakWindow {
  uint32_t start_ms;
  uint32_t end_ms;
  BreakType type;
};

// How long before a break ends the clearance test should begin.
// Same value for both break types per your design decision.
// TESTING VALUE — restore to (10UL * 60UL * 1000UL) [10 min] before evaluation
#define CLEARANCE_WINDOW_MS   15000UL   // 15 seconds for testing

// Maximum breaks we support in one day's schedule
#define MAX_BREAKS   10

// ── PUBLIC FUNCTIONS ─────────────────────────────────────────────────────

// Loads a hardcoded test schedule (stand-in for what BLE will deliver later)
void schedule_load_test_data();

// Loads a schedule received from the app (called from BLE handler in
// Session 2). For now unused, but the signature is fixed so BLE code
// can call straight into it later without touching this file again.
void schedule_load_from_app(BreakWindow* windows, int count);

// Returns true if, based on elapsed shift time, a break is currently active.
// If true, fills *out_window with the active break's details.
bool schedule_is_break_active(uint32_t elapsed_ms, BreakWindow* out_window);

// Returns true if we are within CLEARANCE_WINDOW_MS of the given break's end.
bool schedule_is_clearance_time(uint32_t elapsed_ms, const BreakWindow& window);

#endif