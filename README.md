# Zyntra — Post-Break Physiological Readiness System

**Zyntra** is a biometric readiness evaluation system built for athlete and gym performance testing. It captures physiological baselines (**HRV RMSSD**, **Skin Temperature**, and **Tactile Reaction Time**) using an ESP32 wearable wristband and streams live telemetry via WiFi + MQTT to a React Native web/mobile dashboard.

---

## 📁 Repository Structure

```
Zyntra/
├── Zyntra_Sem5/         # ESP32 Firmware (PlatformIO / C++)
│   ├── src/             # Main loop, HRV, Temperature, Reaction Test, MQTT logic
│   ├── include/         # Header declarations and configuration
│   └── platformio.ini   # PlatformIO build configuration
└── ZyntraApp/           # Mobile & Web Application (React Native / Expo / TypeScript)
    ├── src/app/         # App screens (Users tab, Test tab, Results tab, Recovery, Clearance)
    ├── src/services/    # MQTT link and Context state provider
    └── package.json     # Node dependencies & Expo config
```

---

## ⚡ Features

1. **User Profile Management**: Create athlete/user profiles with age, height, weight, and role tracking.
2. **Resting Baseline Capture**:
   - 20-second HRV RMSSD baseline capture using MAX30102 PPG sensor.
   - Infrared body temperature baseline via MLX90614 sensor.
   - 5-stimulus tactile vibration reaction time baseline via wristband button.
3. **Live Recovery Window**:
   - Real-time vitals streaming over MQTT (`zyntra/vitals`).
   - Live progress display with countdown timer and signal status.
4. **Tri-Modal AND-Gate Clearance Verdict**:
   - Evaluates HRV recovery, skin temperature deviation, and reaction time simultaneously.
   - Displays clear **"Ready"** or **"Not Ready"** verdict with specific signal breakdown and required recovery time.
5. **Session History**:
   - Persists resting baselines and past test results per user.

---

## 🛠️ Hardware Requirements

- **ESP32-WROOM-32** development board
- **MAX30102** Pulse Oximeter & Heart-Rate Sensor (I2C)
- **MLX90614** Non-Contact Infrared Temperature Sensor (I2C)
- **SSD1306** 128x32 OLED Display (I2C)
- Vibration motor & tactile push button (GPIO 18 / 19)

---

## 🚀 Quick Start

### ESP32 Firmware (`Zyntra_Sem5`)
1. Open `Zyntra_Sem5` in VS Code with PlatformIO extension.
2. Update WiFi credentials in `include/config.h` if needed:
   ```cpp
   #define WIFI_SSID "iPhone."
   #define WIFI_PASS "hdd11904"
   #define MQTT_SERVER "broker.hivemq.com"
   ```
3. Connect ESP32 via USB and upload:
   ```bash
   pio run --target upload
   ```

### Mobile / Web App (`ZyntraApp`)
1. Navigate to `ZyntraApp`:
   ```bash
   cd ZyntraApp
   npm install
   ```
2. Start the development server:
   ```bash
   npx expo start --web
   ```
3. Open `http://localhost:8081` in your browser.

---

## 📡 MQTT Topics

- `zyntra/state`: Device state notifications (`BASELINE`, `SHIFT`, `RECOVERY`, `CLEARED`, `NOT_CLEARED`)
- `zyntra/vitals`: Real-time HRV & temperature telemetry during recovery
- `zyntra/baseline`: Published baseline metrics
- `zyntra/result`: Final clearance verdict payload
- `zyntra/command`: Commands sent from App to ESP32 (`START_BASELINE`, `START_RECOVERY`, `ACK`)
