#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <Arduino.h>

// Initialise the MLX90614 sensor
// Returns true if sensor found, false if not
bool temperature_init();

// Capture baseline temperature at shift start
// Takes 5 readings over 2.5 minutes and averages them
// Call this once when the worker first puts on the device
void temperature_capture_baseline();

// Manually set baseline to a specific value (used for testing)
void temperature_set_baseline(float value);

// Read current wrist skin temperature in Celsius
float temperature_read();

// Get the stored baseline temperature
float temperature_get_baseline();

// Check if temperature has recovered to within 0.8 degrees of baseline
// Returns true if cleared, false if still too far from baseline
bool temperature_is_cleared();

// Get current deviation from baseline in Celsius
float temperature_get_deviation();

#endif