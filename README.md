# Zyntra -- Post-Break Physiological Readiness System

**Zyntra** is a multi-modal biometric readiness evaluation system designed for athlete performance testing and industrial safety clearance. It captures physiological baselines (**HRV RMSSD**, **Skin Temperature**, and **Tactile Reaction Time**) using an ESP32 wearable wristband and streams live telemetry via WiFi + MQTT to a React Native web/mobile dashboard.

Clearance decisions apply **age-bracketed dynamic physiological thresholds** derived from large-scale reference studies (*Lifelines cohort*, *Nunan et al. 2010*, *Blomkvist et al. 2017*, *Kosinski 2008*) using a strict **tri-modal AND-gate**.

---

##  Repository Structure

```
Zyntra/
├── Zyntra_Sem5/         # ESP32 Firmware (PlatformIO / C++)
│   ├── src/             # Main loop, HRV, Temperature, Reaction Test, MQTT logic, Clearance algorithm
│   ├── include/         # Header declarations and age-dependent threshold configuration
│   └── platformio.ini   # PlatformIO build configuration
└── ZyntraApp/           # Mobile & Web Application (React Native / Expo / TypeScript)
    ├── src/app/         # App screens (Users dashboard, Test runner, Live telemetry, Captured results)
    ├── src/services/    # MQTT link, storage presets, and Context state provider
    └── package.json     # Node dependencies & Expo config
```

---

##  Features & Enhancements

1. **User Profile & Participant Management**:
   - Track user profiles with age, height, weight, and role classification.
   - Pre-loaded physical gym study dataset for participants (**Chethiya**, **Bhanu**, **Usitha**, **Thilanka**, **Sehath**) with exact resting baselines and post-workout recovery test signals.

2. **Age-Bracketed Dynamic Thresholds**:
   - **HRV (RMSSD)**: Age-adjusted resting reference baselines (e.g. median 52 ms for age 18–24, scaling by age bracket). Requires recovery RMSSD $\ge 90\%$ of worker's baseline.
   - **Skin Temperature**: Dynamic temperature rise threshold ($\Delta T \le 0.8^\circ\text{C}$ for age $\le 45$, $\Delta T \le 0.6^\circ\text{C}$ for age $> 45$).
   - **Vibrotactile Reaction Time**: Age-dependent fatigue cutoff (e.g. $< 450\text{ ms}$ for age 18–25, scaling up to $< 650\text{ ms}$ for age 65+).

3. **Tri-Modal AND-Gate Clearance Logic**:
   - Evaluates HRV recovery, skin temperature deviation, and reaction time latency simultaneously.
   - Displays clear **"READY"** or **"NOT READY"** verdicts with individual signal pass/fail badges.

4. **Realistic Time-to-Clearance Estimation**:
   - Computes individualized recovery time estimates (3 mins per 10% HRV deficit + 5 mins per 0.5°C temp excess), bounded strictly between **5 and 30 minutes**.

5. **Live Recovery Window & MQTT Link**:
   - Real-time vitals streaming over MQTT (`zyntra/vitals`).
   - Bi-directional JSON commands (`START_RECOVERY` passing `userAge` payload). Non-blocking safety timer prevents UI reload hangs.

---

##  Physical Gym Study Participants Dataset (Age Group 18–25)

| Participant | Age | Role | Baseline Signals | Post-Workout Signals | Signal Verdicts | Final Verdict | Est. Recovery Time |
|---|---|---|---|---|---|---|---|
| **Chethiya** | 21 | Athlete / Bodybuilder | HRV: 54.2 ms, Temp: 33.4 °C, RT: 215 ms | HRV: 51.8 ms, Temp: Δ0.4 °C, RT: 495 ms | HRV: PASS, Temp: PASS, RT: **FAIL** | ❌ **NOT READY** | **5 min** |
| **Bhanu** | 24 | Powerlifter | HRV: 58.0 ms, Temp: 33.2 °C, RT: 228 ms | HRV: 54.5 ms, Temp: Δ0.4 °C, RT: 482 ms | HRV: PASS, Temp: PASS, RT: **FAIL** | ❌ **NOT READY** | **8 min** |
| **Usitha** | 23 | Fitness Enthusiast | HRV: 62.5 ms, Temp: 32.8 °C, RT: 210 ms | HRV: 58.0 ms, Temp: Δ0.3 °C, RT: 465 ms | HRV: PASS, Temp: PASS, RT: **FAIL** | ❌ **NOT READY** | **6 min** |
| **Thilanka** | 24 | Crossfit Athlete | HRV: 66.0 ms, Temp: 33.3 °C, RT: 235 ms | HRV: 48.3 ms, Temp: Δ1.2 °C, RT: 357 ms | HRV: **FAIL**, Temp: **FAIL**, RT: PASS | ❌ **NOT READY** | **12 min** |
| **Sehath** | 21 | Endurance Trainer | HRV: 52.0 ms, Temp: 33.5 °C, RT: 220 ms | HRV: 48.8 ms, Temp: Δ1.2 °C, RT: 475 ms | HRV: PASS, Temp: **FAIL**, RT: **FAIL** | ❌ **NOT READY** | **10 min** |

---

##  Hardware Requirements

- **ESP32-WROOM-32** development board
- **MAX30102** Pulse Oximeter & Heart-Rate Sensor (I2C)
- **MLX90614** Non-Contact Infrared Temperature Sensor (I2C)
- **SSD1306** 128x64 OLED Display (I2C)
- Vibration motor & tactile push button (GPIO 18 / 19)

---

##  Quick Start

### ESP32 Firmware (`Zyntra_Sem5`)
1. Open `Zyntra_Sem5` in VS Code with PlatformIO extension.
2. Build & upload firmware to ESP32:
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

##  MQTT Topics & Payloads

- `zyntra/state`: Device state notifications (`BASELINE`, `SHIFT`, `RECOVERY`, `CLEARED`, `NOT_CLEARED`)
- `zyntra/vitals`: Real-time HRV & temperature telemetry during recovery
- `zyntra/baseline`: Published baseline metrics
- `zyntra/result`: Final clearance verdict payload
- `zyntra/command`: Commands sent from App to ESP32 (`START_BASELINE`, `START_RECOVERY` with `userAge`, `ACK`)
