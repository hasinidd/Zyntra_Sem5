#include "schedule.h"

static BreakWindow schedule[MAX_BREAKS];
static int schedule_count = 0;

// ── Load test schedule ──────────────────────────────────────────────────
// For testing without the app/BLE connected yet.
// Times are elapsed milliseconds since shift start (i.e. since setup() ran).
// This mirrors what a real schedule will look like once BLE delivers it —
// just with short testing-scale durations instead of a full 8-hour shift.
//
// Test day plan (elapsed seconds since SHIFT state begins):
//   0:00–0:30  -> working (SHIFT)
//   0:30–1:00  -> "15-min" break   (clearance window: last 15s, i.e. 0:45–1:00)
//   1:00–1:30  -> working (SHIFT)
//   1:30–3:00  -> "1-hr" break     (clearance window: last 15s, i.e. 2:45–3:00)
//
// NOTE: these are testing-scale (seconds, not real 15/60 minutes), and
// CLEARANCE_WINDOW_MS is scaled down to match (15s instead of 10min) so
// you can see the full SHIFT -> RECOVERY -> CLEARANCE flow in about three
// minutes instead of over an hour. Swap in real millisecond values in both
// this file and schedule.h before the evaluation demo.
void schedule_load_test_data() {
  schedule_count = 0;

  // Short break: starts at 30s, ends at 60s — 30s long, so the 15s
  // clearance window (starting at 45s) fits comfortably inside it
  schedule[schedule_count].start_ms = 30000;
  schedule[schedule_count].end_ms   = 60000;
  schedule[schedule_count].type     = BREAK_SHORT;
  schedule_count++;

  // Long break: starts at 90s, ends at 180s — 90s long
  schedule[schedule_count].start_ms = 90000;
  schedule[schedule_count].end_ms   = 180000;
  schedule[schedule_count].type     = BREAK_LONG;
  schedule_count++;

  Serial.println("[SCHEDULE] Test schedule loaded:");
  for (int i = 0; i < schedule_count; i++) {
    Serial.printf("  Break %d: %lu ms -> %lu ms (%s)\n",
      i,
      schedule[i].start_ms,
      schedule[i].end_ms,
      schedule[i].type == BREAK_SHORT ? "SHORT" : "LONG");
  }
}

// ── Load schedule from app (Session 2 will call this from BLE) ───────────
void schedule_load_from_app(BreakWindow* windows, int count) {
  if (count > MAX_BREAKS) count = MAX_BREAKS;
  schedule_count = count;
  for (int i = 0; i < schedule_count; i++) {
    schedule[i] = windows[i];
  }
  Serial.printf("[SCHEDULE] Loaded %d break(s) from app\n", schedule_count);
}

// ── Is a break active right now? ──────────────────────────────────────────
bool schedule_is_break_active(uint32_t elapsed_ms, BreakWindow* out_window) {
  for (int i = 0; i < schedule_count; i++) {
    if (elapsed_ms >= schedule[i].start_ms && elapsed_ms < schedule[i].end_ms) {
      if (out_window != nullptr) {
        *out_window = schedule[i];
      }
      return true;
    }
  }
  return false;
}

// ── Are we within the clearance window before this break ends? ───────────
bool schedule_is_clearance_time(uint32_t elapsed_ms, const BreakWindow& window) {
  // Guard: don't fire if the break is shorter than the clearance window
  // itself (shouldn't happen with real 15/60 min breaks, but protects
  // against bad schedule data).
  if (window.end_ms < CLEARANCE_WINDOW_MS) return false;

  uint32_t clearance_start = window.end_ms - CLEARANCE_WINDOW_MS;
  return elapsed_ms >= clearance_start && elapsed_ms < window.end_ms;
}