#ifndef CONFIG_H
#define CONFIG_H

// ── PIN DEFINITIONS ───────────────────────────────────────────────────────
#define PIN_MOTOR         4    // Vibration motor via transistor
#define PIN_BUTTON       15    // Tactile button (INPUT_PULLUP)
#define PIN_LED           2    // Onboard LED for debug/status
#define PIN_SDA          21    // I2C data
#define PIN_SCL          22    // I2C clock

// ── I2C ADDRESSES ─────────────────────────────────────────────────────────
#define ADDR_MAX30102  0x57
#define ADDR_MLX90614  0x5A
#define ADDR_SSD1306   0x3C

// ── BASELINE CAPTURE ──────────────────────────────────────────────────────
// Real value: 180000 (3 min). 
// 90000 = 1.5 min — enough for wrist HRV with custom peak detector
#define BASELINE_DURATION_MS       90000  // 1.5 min
#define BASELINE_TEMP_READINGS         5  // Number of temp readings to average

// ── RECOVERY MODE ─────────────────────────────────────────────────────────
// Real value: 600000 (10 min). TESTING VALUE active below.
#define BREAK_DURATION_MS         120000  // 2 min for testing (legacy)

#define HRV_POLL_INTERVAL_MS       60000  // Sample HRV every 60 seconds
#define TEMP_POLL_INTERVAL_MS      30000  // Sample temperature every 30 seconds
#define OLED_UPDATE_INTERVAL_MS     1000  // Refresh display every 1 second

// ── CLEARANCE THRESHOLDS ──────────────────────────────────────────────────
#define HRV_RECOVERY_THRESHOLD      0.90  // Must reach 90% of baseline RMSSD
#define TEMP_RECOVERY_MARGIN_C       0.8  // Must be within 0.8C of baseline
#define RT_THRESHOLD_MS              500  // Median RT must be under 500ms

// ── RETRY LOGIC ───────────────────────────────────────────────────────────
#define RETRY_EXTENSION_MS         15000UL  // 15 sec for testing
#define MAX_RETRY_ATTEMPTS              3

// ── BUTTON HANDLING ───────────────────────────────────────────────────────
#define BUTTON_DEBOUNCE_MS           300
#define BUTTON_HOLD_MS              1000

// ── WIFI & MQTT CONFIGURATION ─────────────────────────────────────────────
#define WIFI_SSID                  "iPhone."        // User's iPhone hotspot SSID
#define WIFI_PASS                  "hdd11904"       // User's iPhone hotspot Password
#define MQTT_SERVER                "broker.hivemq.com" // Free public broker or local IP
#define MQTT_PORT_TCP              1883
#define MQTT_CLIENT_ID             "Zyntra_ESP32_01"

// Topics
#define MQTT_TOPIC_STATE           "zyntra/state"
#define MQTT_TOPIC_VITALS          "zyntra/vitals"
#define MQTT_TOPIC_BASELINE        "zyntra/baseline"
#define MQTT_TOPIC_RESULT          "zyntra/result"
#define MQTT_TOPIC_COMMAND         "zyntra/command"

// Commands from App
#define MQTT_CMD_START_BASELINE    0x01
#define MQTT_CMD_START_RECOVERY    0x02
#define MQTT_CMD_ACK               0x03

#endif