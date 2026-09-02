#ifndef HRV_H
#define HRV_H

#include <Arduino.h>

// Initialise the MAX30102 sensor
// Returns true if found, false if not
bool hrv_init();

// Call this every loop iteration — feeds raw IR sample into beat detector
// Must be called as frequently as possible for accurate peak detection
void hrv_process_sample();

// Compute RMSSD from stored RR intervals
// Returns RMSSD in milliseconds, or -1 if not enough data
float hrv_compute_rmssd();

// Capture HRV baseline at shift start
// Collects RR intervals for 3 minutes and computes baseline RMSSD
void hrv_capture_baseline();

// Get stored baseline RMSSD
float hrv_get_baseline();

// Check if HRV has recovered to >= 90% of baseline
bool hrv_is_cleared();

// Get current RMSSD as percentage of baseline
float hrv_get_recovery_percent();

// Get current heart rate in BPM (for display purposes)
float hrv_get_bpm();

#endif