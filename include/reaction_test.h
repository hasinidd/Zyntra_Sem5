#ifndef REACTION_TEST_H
#define REACTION_TEST_H

#include <Arduino.h>

// Initialise pins for vibration motor and button
void rt_init();

// Run the full 5-stimulus reaction time test
// Returns median reaction time in milliseconds
// Returns 9999 if test failed (too many timeouts)
uint16_t rt_run_test();

// Get the last recorded median RT in ms
uint16_t rt_get_last_median();

// Check if RT has passed the clearance threshold (under 500ms)
bool rt_is_cleared();

#endif