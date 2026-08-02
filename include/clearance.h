#ifndef CLEARANCE_H
#define CLEARANCE_H

#include <Arduino.h>

// Struct holding the full clearance result
struct ClearanceResult {
  bool hrv_pass;            // True if HRV recovered to threshold
  bool temp_pass;           // True if temperature within margin
  bool rt_pass;             // True if reaction time under threshold
  bool cleared;             // True only if all three pass (AND-gate)
  bool hrv_data_valid;      // False if HRV could not be measured at all
  float hrv_current;        // Current RMSSD in ms (-1 if invalid)
  float hrv_baseline;       // Morning baseline RMSSD in ms
  float temp_deviation;     // Deviation from baseline in Celsius
  uint16_t rt_median;       // Median reaction time in ms
  int minutes_to_clearance; // Estimated minutes until clearance
};

// Run the full clearance protocol
// Checks all three signals and returns a ClearanceResult
ClearanceResult clearance_run(float hrv_baseline, float temp_baseline);

// Estimate minutes to clearance based on recovery curve
int clearance_estimate_minutes(float hrv_current, float hrv_baseline,
                                float temp_deviation);

#endif