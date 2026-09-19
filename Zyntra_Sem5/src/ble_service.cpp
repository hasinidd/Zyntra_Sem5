// Legacy BLE service stub — replaced by WiFi + MQTT (mqtt_service.cpp)
#include "ble_service.h"

bool ble_init() { return false; }
void ble_notify_state(uint8_t state) {}
void ble_notify_vitals(int16_t rmssd, int16_t temp_baseline, int16_t temp_current, uint16_t seconds_remaining) {}
void ble_notify_result(const ClearanceResult& result) {}
uint8_t ble_get_command() { return 0; }
bool ble_is_connected() { return false; }