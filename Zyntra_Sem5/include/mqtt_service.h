#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include <Arduino.h>
#include "clearance.h"

// ── State enum mapping for MQTT ───────────────────────────────────────────
#define MQTT_STATE_DISCONNECTED  0
#define MQTT_STATE_BASELINE      1
#define MQTT_STATE_SHIFT         2
#define MQTT_STATE_RECOVERY      3
#define MQTT_STATE_CLEARANCE     4
#define MQTT_STATE_CLEARED       5
#define MQTT_STATE_NOT_CLEARED   6

// Initialise WiFi and MQTT client
bool mqtt_init();

// Maintain MQTT connection and process incoming messages — call in loop()
void mqtt_loop();

// Notify tablet/web app of current device state
void mqtt_notify_state(uint8_t state);

// Notify tablet/web app of live vitals during recovery (called every second)
void mqtt_notify_vitals(int16_t rmssd, int16_t temp_baseline,
                        int16_t temp_current, uint16_t seconds_remaining);

// Notify tablet/web app when baseline capture completes
void mqtt_notify_baseline(float hrv_rmssd, float temp_c, uint16_t rt_ms);

// Notify tablet/web app of final clearance result
void mqtt_notify_result(const ClearanceResult& result);

// Check if a command was received from app — returns command byte or 0
uint8_t mqtt_get_command();

// Check if MQTT client is connected
bool mqtt_is_connected();

#endif
