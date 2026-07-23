# Zyntra — Weekly Project Plan

**Project:** Post-Break Physiological Readiness Clearance System  
**Module:** CS3283 Embedded Systems Project | Semester 5 | University of Moratuwa  
**Mid Evaluation:** 17 August 2026 | **Final Evaluation:** 5 October 2026

---

## Summary

| Week | Dates | Focus | Status |
|---|---|---|---|
| Week 3 | Jul 21–27 | Hardware verification + HRV + Temp + OLED firmware | 🟡 In Progress |
| Week 4 | Jul 28–Aug 03 | RT test + Clearance algorithm + State machine + BLE | ⬜ Upcoming |
| Week 5 | Aug 04–10 | Firmware polish + Perfboard + React Native + BLE | ⬜ Upcoming |
| Week 6 | Aug 11–17 | Mid eval prep + Complete React Native screens | ⬜ Upcoming |
| **MID EVAL** | **Aug 17** | **Hardware + Firmware + BLE app demonstration** | 🎯 Target |
| Week 7 | Aug 18–24 | PostgreSQL locally + FastAPI backend | ⬜ Upcoming |
| Week 8 | Aug 25–31 | AWS account + RDS PostgreSQL cloud | ⬜ Upcoming |
| Week 9 | Sep 01–07 | AWS Lambda + API Gateway + SNS | ⬜ Upcoming |
| Week 10 | Sep 08–14 | Sensor calibration + Validation sessions 1–5 | ⬜ Upcoming |
| Week 11 | Sep 15–21 | Validation sessions 6–15 + Results analysis | ⬜ Upcoming |
| Week 12 | Sep 22–28 | Hardening + Battery test + Report + Demo video | ⬜ Upcoming |
| Week 13 | Sep 29–Oct 05 | Final rehearsal + Final evaluation | ⬜ Upcoming |
| **FINAL EVAL** | **Oct 5** | **Full system + Cloud + Validation results** | 🎯 Target |

---

## Detailed Weekly Breakdown

<details>
<summary><strong>Week 3 — Jul 21–27 — Hardware Verification + Sensor Firmware</strong> 🟡</summary>

### Slot 1 ✅ — Hardware Verification
**Goal:** Confirm all sensors are correctly wired and detected by ESP32

**Tasks:**
- Set up PlatformIO project with `platformio.ini` configured for ESP32
- Install all 5 libraries via `pio lib install`
- Create all firmware file skeletons in `src/` and `include/`
- Write and run I2C scanner to verify all three sensors

**Done when:**
- Serial monitor shows 0x57 (MAX30102), 0x5A (MLX90614), 0x3C (OLED) on every scan

---

### Slot 2 🟡 — HRV + Temperature + OLED Firmware
**Goal:** Get real sensor data reading and displaying on OLED

**Tasks:**
- Configure MAX30102 at 400Hz — implement `checkForBeat()` — extract RR intervals
- Compute RMSSD from 2-minute sliding window
- Implement MLX90614 baseline capture — average 5 readings 30 seconds apart
- Build all four OLED screen layouts

**Done when:**
- RMSSD updates every 60 seconds on serial monitor with finger on MAX30102
- MLX90614 reads within ±0.5°C of a reference thermometer
- All four OLED screens display correctly

</details>

---

<details>
<summary><strong>Week 4 — Jul 28–Aug 03 — RT Test + Clearance Algorithm + BLE</strong> ⬜</summary>

### Slot 3 — RT Test + AND-Gate Clearance
**Goal:** Full clearance protocol running end to end

**Tasks:**
- Implement 5-stimulus vibration test with random intervals (3–8 seconds)
- Add false start detection (under 100ms rejected)
- Add timeout handling (no response within 2000ms = 9999ms)
- Compute median of 5 valid responses
- Implement AND-gate: HRV_pass AND Temp_pass AND RT_pass = CLEARED
- Implement time-to-clearance linear regression estimate

**Done when:**
- NOT READY shows with exact failed signal named on OLED
- READY shows after full recovery
- Both outcomes verified at least 3 times each

---

### Slot 4 — State Machine + BLE GATT Server
**Goal:** Full state machine + BLE advertising confirmed

**Tasks:**
- Implement 5-state machine: BASELINE → SHIFT → RECOVERY → RT_TEST → CLEARED/NOT_CLEARED
- Implement BLE GATT server with custom Service UUID and Characteristic UUID
- Broadcast JSON on every recovery update (60s) and on every verdict
- Verify BLE advertisement using nRF Connect app on phone

**Done when:**
- Device moves through all five states in correct order
- nRF Connect shows Zyntra_01 in scan
- JSON payload readable in nRF Connect when subscribed

</details>

---

<details>
<summary><strong>Week 5 — Aug 04–10 — Firmware Polish + React Native + BLE</strong> ⬜</summary>

### Slot 5 — Firmware Polish + Perfboard
**Goal:** Stable physical build with polished firmware

**Tasks:**
- Add signal quality check (IR below 50000 = show "Adjust device" on OLED)
- Add RR artifact filtering (reject intervals deviating >20% from previous)
- Add time-to-clearance estimate on NOT READY screen
- Solder all components from breadboard to perfboard
- Run passive current draw test to estimate battery life

**Done when:**
- Perfboard build survives being lifted and moved without failures
- Estimated battery life exceeds 10 hours
- Signal quality warning appears when finger removed from MAX30102

---

### Slot 6 — React Native + BLE Connection
**Goal:** Live BLE data showing on supervisor phone

**Tasks:**
- Create project: `npx react-native init ZyntraApp`
- Install `react-native-ble-plx`
- Add BLUETOOTH_SCAN, BLUETOOTH_CONNECT, ACCESS_FINE_LOCATION to AndroidManifest.xml
- Write BleService.js — scan, connect, discover GATT, subscribe, decode Base64 JSON
- Build RecoveryScreen with three animated progress bars

**Done when:**
- Phone shows live HRV% and temperature deviation matching OLED simultaneously
- Values update every 60 seconds without manual refresh
- BLE reconnects automatically if phone moves out of range

</details>

---

<details>
<summary><strong>Week 6 — Aug 11–17 — Mid Evaluation Preparation 🎯</strong> ⬜</summary>

### Slot 7 — Complete React Native Screens + Demo Rehearsal
**Goal:** Full demo ready for mid evaluation

**Tasks:**
- Build ClearanceResultScreen — READY (green) / NOT READY (red) with per-signal pass/fail
- Build DashboardScreen — worker status card with colour-coded state
- Rehearse full demo at least 3 times
- Deliberately trigger NOT READY to demonstrate failure path
- Record 3-minute backup demo video

**Done when:**
- Full demo runs start to finish in under 20 minutes without errors
- Both READY and NOT READY demonstrated on OLED and phone simultaneously
- Backup video recorded and saved

---

### 🎯 Mid Evaluation — August 17

**What to demonstrate:**
- Wristband baseline capture (3 minutes resting)
- Break triggered by button press
- Live recovery monitoring on OLED and phone simultaneously
- RT test — 5 vibration stimuli, worker taps each time
- READY verdict on both OLED and phone
- NOT READY scenario with failed signal identified
- System architecture explained

**Not needed for mid eval:**
- AWS backend
- PostgreSQL database
- Push notifications
- Validation study

</details>

---

<details>
<summary><strong>Week 7 — Aug 18–24 — PostgreSQL + FastAPI Backend</strong> ⬜</summary>

### Slot 8 — PostgreSQL Locally
**Tasks:**
- Install PostgreSQL via Docker
- Create `workers`, `shift_baselines`, `clearance_events` tables
- Test insertions and queries in psql

**Done when:** Worker → baseline → clearance event chain works with correct JOIN queries

---

### Slot 9 — FastAPI REST API
**Tasks:**
- POST /workers — register new worker
- POST /clearance-events — save clearance result
- GET /workers/:id/events — all events for one worker
- GET /events/summary — total events and clearance rate
- GET /clearance-events/latest — most recent event per worker
- Connect React Native ApiService.js to local FastAPI

**Done when:** Real clearance event from ESP32 flows through BLE → React Native → FastAPI → PostgreSQL

</details>

---

<details>
<summary><strong>Week 8 — Aug 25–31 — AWS Account + RDS PostgreSQL</strong> ⬜</summary>

### Slot 10 — AWS Setup
**Tasks:**
- Create AWS account and IAM user with programmatic access
- Install AWS CLI and configure credentials
- Create RDS PostgreSQL instance (db.t3.micro free tier)

**Done when:** Connect to RDS via psql from local machine without errors

---

### Slot 11 — Migrate to RDS
**Tasks:**
- Export local schema with `pg_dump --schema-only`
- Import to RDS via psql
- Update FastAPI `DATABASE_URL` to RDS endpoint
- Confirm React Native saves events to cloud database

**Done when:** Clearance event from ESP32 saves to AWS RDS successfully

</details>

---

<details>
<summary><strong>Week 9 — Sep 01–07 — Lambda + API Gateway + SNS</strong> ⬜</summary>

### Slot 12 — AWS Lambda + API Gateway
**Tasks:**
- Add Mangum adapter to FastAPI (`handler = Mangum(app)`)
- Package backend as ZIP and deploy to Lambda
- Create API Gateway REST routes pointing to Lambda
- Update React Native to call API Gateway URL

**Done when:** All five endpoints respond via API Gateway URL with correct responses

---

### Slot 13 — SNS Push Notifications + Audit Log
**Tasks:**
- Create SNS topic `zyntra-alerts`
- Lambda triggers SNS when cleared=false
- Subscribe supervisor phone to SNS topic
- Build AuditLogScreen with CSV export

**Done when:** Full chain works — ESP32 → BLE → React Native → API Gateway → Lambda → RDS → SNS push notification

</details>

---

<details>
<summary><strong>Week 10 — Sep 08–14 — Sensor Calibration + Validation Begins</strong> ⬜</summary>

### Slot 14 — Sensor Calibration
**Tasks:**
- Compare MAX30102 RMSSD against Elite HRV app — 10 paired measurements
- Compare MLX90614 against medical thermometer — 10 paired readings
- Document results in `docs/calibration.md`

**Done when:** MAX30102 within ±8ms RMSSD · MLX90614 within ±0.5°C vs references

---

### Slot 15 — Validation Sessions 1–5 (Volunteer 1)
**Protocol per session:**
- 3-minute baseline rest
- 20-minute treadmill at 80% max heart rate
- 15-minute break with Zyntra monitoring
- RT test + PVT simultaneously at break end
- Record all values in CSV

**Done when:** 5 complete sessions recorded with device verdict and PVT ground truth

</details>

---

<details>
<summary><strong>Week 11 — Sep 15–21 — Complete Validation + Results Analysis</strong> ⬜</summary>

### Slot 16 — Validation Sessions 6–15
**Tasks:**
- Volunteer 2 — 5 sessions
- Volunteer 3 — 5 sessions
- Same protocol as Volunteer 1

**Done when:** 15 complete rows in CSV — all volunteers, all sessions, no missing values

---

### Slot 17 — Results Analysis
**Tasks:**
- Run `compute_metrics.py` on 15-session CSV
- Compute sensitivity, specificity, accuracy, PPV, NPV
- Generate confusion matrix
- Plot HRV and temperature recovery curves

**Done when:**
- Sensitivity and specificity values computed
- Confusion matrix generated
- Summary: "Zyntra correctly cleared X% of ready workers and withheld clearance from Y% of unready workers"

</details>

---

<details>
<summary><strong>Week 12 — Sep 22–28 — Hardening + Report + Demo Video</strong> ⬜</summary>

### Slot 18 — Hardening + Battery Test
**Tasks:**
- Run 12-hour continuous battery life test
- Assemble final wristband housing
- Drop test from 1 metre — confirm I2C scanner still shows all sensors
- Water splash test — confirm no reset

**Done when:** 10+ hours battery life · passes drop and water tests

---

### Slot 19 — Project Report + Demo Video
**Tasks:**
- Record 5-minute demo video covering full system flow
- Write complete project report (problem → design → firmware → app → cloud → validation → results)
- Update GitHub wiki with all weekly entries

**Done when:** Report complete · demo video recorded · GitHub wiki up to date

</details>

---

<details>
<summary><strong>Week 13 — Sep 29–Oct 05 — Final Evaluation 🎯</strong> ⬜</summary>

### Slot 20 — Final Rehearsal
**Tasks:**
- Run complete demonstration 3 times
- Practice answering Q&A questions
- Verify all GitHub commits meaningful and wiki complete

**Done when:** All rehearsal runs error-free · Q&A answers fluent

---

### 🎯 Final Evaluation — October 5

**What to demonstrate:**
- Live wristband clearance protocol end to end
- React Native app receiving BLE data
- AWS cloud pipeline: API Gateway → Lambda → RDS saving events
- NOT READY triggering SNS push notification on phone
- Audit log with event history and CSV export
- Validation results: sensitivity, specificity, confusion matrix
- Novelty claim and publication target explained

**Full checklist:**
- Hardware ✅ Firmware ✅ React Native ✅
- AWS API Gateway ✅ Lambda ✅ RDS PostgreSQL ✅ SNS ✅
- 15 validation sessions ✅ Sensitivity/specificity ✅
- GitHub complete ✅ Research publication angle ✅

</details>