#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <Arduino.h>
#include "clearance.h"

// ── BLE Service for Zyntra ────────────────────────────────────────────────
// UUIDs match config.h exactly — do not change here without changing config.h
//
// Service:    12345678-1234-1234-1234-123456789012
// Characteristics:
//   STATE     - notify  - current device state (1 byte)
//   VITALS    - notify  - live HRV + temp during recovery (8 bytes)
//   RESULT    - notify  - clearance verdict + signal values (12 bytes)
//   COMMAND   - write   - supervisor commands (1 byte)
//
// State values (1 byte):
//   0 = DISCONNECTED
//   1 = BASELINE
//   2 = SHIFT
//   3 = RECOVERY
//   4 = CLEARANCE
//   5 = CLEARED
//   6 = NOT_CLEARED
//
// Command values (1 byte):
//   0x01 = trigger break (supervisor-side break start)
//   0x02 = acknowledge result (return to SHIFT)

#define BLE_STATE_DISCONNECTED  0
#define BLE_STATE_BASELINE      1
#define BLE_STATE_SHIFT         2
#define BLE_STATE_RECOVERY      3
#define BLE_STATE_CLEARANCE     4
#define BLE_STATE_CLEARED       5
#define BLE_STATE_NOT_CLEARED   6

#define BLE_CMD_TRIGGER_BREAK   0x01
#define BLE_CMD_ACKNOWLEDGE     0x02

// Characteristic UUIDs
#define BLE_CHAR_STATE_UUID   "aaaaaaaa-1234-1234-1234-123456789001"
#define BLE_CHAR_VITALS_UUID  "aaaaaaaa-1234-1234-1234-123456789002"
#define BLE_CHAR_RESULT_UUID  "aaaaaaaa-1234-1234-1234-123456789003"
#define BLE_CHAR_CMD_UUID     "aaaaaaaa-1234-1234-1234-123456789004"

// Initialise BLE — call once in setup()
bool ble_init();

// Notify tablet of current device state
void ble_notify_state(uint8_t state);

// Notify tablet of live vitals during recovery (called every second)
// rmssd: current RMSSD in ms * 10 (e.g. 803 = 80.3ms), -1 if invalid
// temp_baseline: baseline temp in C * 10
// temp_current: current temp in C * 10
// seconds_remaining: seconds left in recovery window
void ble_notify_vitals(int16_t rmssd, int16_t temp_baseline,
                       int16_t temp_current, uint16_t seconds_remaining);

// Notify tablet of clearance result
void ble_notify_result(const ClearanceResult& result);

// Check if a command was received from tablet — call in loop()
// Returns 0 if no command, otherwise the command byte
uint8_t ble_get_command();

// Check if tablet is connected
bool ble_is_connected();

#endif