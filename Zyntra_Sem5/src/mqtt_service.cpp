#include "mqtt_service.h"
#include "config.h"
#include "temperature.h"
#include "hrv.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

static WiFiClient espClient;
static PubSubClient mqttClient(espClient);

static uint8_t pendingCommand = 0;
static uint8_t participantAge = 24; // Default age 24
static uint32_t lastReconnectAttempt = 0;

static const char* STATE_NAMES[] = {
  "DISCONNECTED", "BASELINE", "SHIFT", "RECOVERY", "CLEARANCE", "CLEARED", "NOT_CLEARED"
};

// ── Callback for incoming MQTT messages ───────────────────────────────────
static void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char message[256] = {0};
  unsigned int copyLen = length < 255 ? length : 255;
  memcpy(message, payload, copyLen);
  message[copyLen] = '\0';

  Serial.print("[MQTT] Message arrived on [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  if (strcmp(message, "START_BASELINE") == 0 || message[0] == '1' || payload[0] == 0x01) {
    pendingCommand = MQTT_CMD_START_BASELINE;
  } else if (strcmp(message, "START_RECOVERY") == 0 || message[0] == '2' || payload[0] == 0x02) {
    pendingCommand = MQTT_CMD_START_RECOVERY;
  } else if (strcmp(message, "ACK") == 0 || message[0] == '3' || payload[0] == 0x03) {
    pendingCommand = MQTT_CMD_ACK;
  } else {
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, message, length);
    if (!err) {
      if (doc.containsKey("command")) {
        const char* cmdStr = doc["command"];
        if (cmdStr && strcmp(cmdStr, "START_RECOVERY") == 0) {
          pendingCommand = MQTT_CMD_START_RECOVERY;
        } else if (cmdStr && strcmp(cmdStr, "START_BASELINE") == 0) {
          pendingCommand = MQTT_CMD_START_BASELINE;
        } else if (cmdStr && strcmp(cmdStr, "ACK") == 0) {
          pendingCommand = MQTT_CMD_ACK;
        } else if (doc["command"].is<uint8_t>()) {
          pendingCommand = doc["command"].as<uint8_t>();
        }
      }

      if (doc.containsKey("userAge")) {
        participantAge = doc["userAge"].as<uint8_t>();
        Serial.print("[MQTT] Participant age set from app: ");
        Serial.println(participantAge);
      }

      if (doc.containsKey("tempBaseline")) {
        float tb = doc["tempBaseline"].as<float>();
        temperature_set_baseline(tb);
      }

      if (doc.containsKey("hrvBaseline")) {
        float hb = doc["hrvBaseline"].as<float>();
        hrv_set_baseline(hb);
      }
    }
  }

  if (pendingCommand > 0) {
    Serial.print("[MQTT] Processed command code: 0x0");
    Serial.println(pendingCommand, HEX);
  }
}

// ── WiFi & MQTT Reconnect logic ───────────────────────────────────────────
static bool reconnectMqtt() {
  if (mqttClient.connected()) return true;

  Serial.print("[MQTT] Reconnecting to ");
  Serial.print(MQTT_SERVER);
  Serial.println("...");

  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("[MQTT] Connected to MQTT broker!");
    mqttClient.subscribe(MQTT_TOPIC_COMMAND);
    return true;
  } else {
    Serial.print("[MQTT] Failed, rc=");
    Serial.println(mqttClient.state());
    return false;
  }
}

bool mqtt_init() {
  Serial.print("[WIFI] Connecting to SSID: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  uint32_t startMs = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startMs < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WIFI] Connected!");
    Serial.print("[WIFI] IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WIFI] Connection timed out — will retry in background");
  }

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT_TCP);
  mqttClient.setCallback(mqttCallback);

  return reconnectMqtt();
}

void mqtt_loop() {
  if (WiFi.status() != WL_CONNECTED) {
    uint32_t now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      WiFi.reconnect();
    }
    return;
  }

  if (!mqttClient.connected()) {
    uint32_t now = millis();
    if (now - lastReconnectAttempt > 2000) {
      lastReconnectAttempt = now;
      reconnectMqtt();
    }
  } else {
    mqttClient.loop();
  }
}

void mqtt_notify_state(uint8_t state) {
  if (!mqttClient.connected()) reconnectMqtt();
  if (!mqttClient.connected()) return;

  StaticJsonDocument<128> doc;
  doc["state"] = state;
  doc["name"]  = (state <= 6) ? STATE_NAMES[state] : "UNKNOWN";

  char buf[128];
  size_t n = serializeJson(doc, buf);
  mqttClient.publish(MQTT_TOPIC_STATE, (const uint8_t*)buf, n, true);

  Serial.print("[MQTT] State published: ");
  Serial.println(buf);
}

void mqtt_notify_vitals(int16_t rmssd, int16_t temp_baseline,
                        int16_t temp_current, uint16_t seconds_remaining) {
  if (!mqttClient.connected()) reconnectMqtt();
  if (!mqttClient.connected()) return;

  StaticJsonDocument<256> doc;
  doc["rmssd"]            = (rmssd < 0) ? (float)-1.0 : (float)rmssd / 10.0;
  doc["tempBaseline"]     = (float)temp_baseline / 10.0;
  doc["tempCurrent"]      = (float)temp_current / 10.0;
  doc["secondsRemaining"] = seconds_remaining;

  char buf[256];
  size_t n = serializeJson(doc, buf);
  mqttClient.publish(MQTT_TOPIC_VITALS, (const uint8_t*)buf, n, false);
}

void mqtt_notify_baseline(float hrv_rmssd, float temp_c, uint16_t rt_ms) {
  if (!mqttClient.connected()) reconnectMqtt();
  if (!mqttClient.connected()) return;

  StaticJsonDocument<256> doc;
  doc["hrvRmssd"]   = hrv_rmssd;
  doc["tempC"]      = temp_c;
  doc["rtMedianMs"] = rt_ms;

  char buf[256];
  size_t n = serializeJson(doc, buf);
  mqttClient.publish(MQTT_TOPIC_BASELINE, (const uint8_t*)buf, n, true);

  Serial.print("[MQTT] Baseline published: ");
  Serial.println(buf);
}

void mqtt_notify_result(const ClearanceResult& r) {
  if (!mqttClient.connected()) reconnectMqtt();
  if (!mqttClient.connected()) return;

  StaticJsonDocument<384> doc;
  doc["cleared"]            = r.cleared;
  doc["hrvDataValid"]       = r.hrv_data_valid;
  doc["hrvPass"]            = r.hrv_pass;
  doc["tempPass"]           = r.temp_pass;
  doc["rtPass"]             = r.rt_pass;
  doc["rmssd"]              = r.hrv_current > 0 ? r.hrv_current : -1.0;
  doc["tempDeltaC"]         = r.temp_deviation;
  doc["medianRtMs"]         = r.rt_median;
  doc["minutesToClearance"] = r.minutes_to_clearance;

  char buf[384];
  size_t n = serializeJson(doc, buf);
  mqttClient.publish(MQTT_TOPIC_RESULT, (const uint8_t*)buf, n, true);

  Serial.print("[MQTT] Result published: ");
  Serial.println(buf);
}

uint8_t mqtt_get_command() {
  uint8_t cmd = pendingCommand;
  pendingCommand = 0;
  return cmd;
}

uint8_t mqtt_get_participant_age() {
  return participantAge;
}

bool mqtt_is_connected() {
  return mqttClient.connected();
}
