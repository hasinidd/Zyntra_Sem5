# Weekly Project Plan

---

## Overview

| Week | Focus | Status |
|---|---|---|
| Week 1 | Hardware verification + HRV + Temperature + OLED firmware | Done |
| Week 2 | React Native screens + Demo preparation + Mid evaluation | Done |
| Week 3 | RT test + Clearance algorithm | Done |
| Week 4 | Expo project setup + Android build pivot | Done |
| Week 5 | React Native UI (Recovery + Clearance screens) + BLE integration on app side | Done |
| Week 6 | AWS account setup + IAM + CLI | Done |
| Week 7 | Firmware polish + Perfboard assembly | Upcoming |
| Week 8 | State machine + BLE GATT server (firmware side) | In Progress |
| Week 9 | PostgreSQL local setup + FastAPI REST backend | Upcoming |
| Week 10 | RDS PostgreSQL cloud migration | Upcoming |
| Week 11 | AWS Lambda + API Gateway + SNS push notifications | Upcoming |
| Week 12 | Sensor calibration + Validation sessions | Upcoming |
| Week 13 | System hardening + Battery test + Project report + Demo video | Upcoming |
| Week 14 | Final rehearsal + Final evaluation | Upcoming |

---

## Weekly Progress

<details>
<summary><strong>Week 1 — Hardware Verification + Sensor Firmware</strong></summary>

### Slot 1 — Environment Setup and Hardware Verification
**Status:** Done

- Initialised PlatformIO project with `platformio.ini` configured for ESP32 Dev Module
- Installed all five firmware libraries via `pio lib install`
- Created complete firmware file skeleton across `src/` and `include/` directories
- Wrote and deployed I2C scanner sketch
- Confirmed all three sensors detected at correct addresses: MAX30102 at 0x57, MLX90614 at 0x5A, SSD1306 OLED at 0x3C

---

### Slot 2 — HRV Module + Temperature Module + OLED Display
**Status:** Done

- Configured MAX30102 for HRV-optimised mode (LED brightness 50, 100Hz sample rate, 411us pulse width — calibrated empirically)
- Implemented beat detection using `checkForBeat()` to extract RR intervals from PPG signal
- Computed RMSSD from a sliding window of up to 140 RR intervals
- Implemented MLX90614 baseline capture and temperature deviation check
- Built all OLED screen layouts: shift mode, recovery progress, READY, NOT READY, sensor error, signal-lost warning, countdown

</details>

---

<details>
<summary><strong>Week 2 — Mid Evaluation</strong></summary>

### Slot 3 — Demo Preparation
**Status:** Done

- Presented mid evaluation using confirmed hardcoded-baseline clearance results (READY / NOT READY verdicts on physical hardware)
- Demonstrated live hardware: sensor readings, tri-modal AND-gate algorithm, and OLED verdict display

---

### Mid Evaluation
**Status:** Completed

**Scope presented:** ESP32 wristband hardware and firmware, with the tri-modal clearance algorithm (HRV, skin temperature, reaction time) demonstrated live using pre-set baseline values.

</details>

---

<details>
<summary><strong>Week 3 — RT Test + Clearance Algorithm</strong></summary>

### Slot 4 — Reaction Time Test + AND-Gate Clearance Logic
**Status:** Done

- Implemented 5-stimulus vibration reaction time test with pseudo-random intervals between 3 and 8 seconds
- Added false start detection (responses under 100ms rejected and repeated)
- Added timeout handling (no response within 2000ms recorded as failed trial)
- Computed median reaction time from 5 valid responses
- Implemented AND-gate clearance logic combining HRV, temperature, and reaction time
- Added three-state HRV handling (PASS / FAIL / INSUFFICIENT DATA) to distinguish sensor errors from genuine physiological failure
- Implemented linear regression time-to-clearance estimate on NOT READY screen
- **Confirmed on physical hardware:** both READY (HRV 165.73ms PASS, Temp 0.20°C PASS, RT 379ms PASS) and NOT READY (HRV 67.30ms FAIL, Temp 0.40°C PASS, RT 346ms PASS) verdicts reproduced successfully

</details>

---

<details>
<summary><strong>Week 4 — Expo Project Setup + Android Build Pivot</strong></summary>

### Slot 5 — React Native (Expo) Project Setup
**Status:** Done

- Created Expo project `ZyntraApp` and installed `react-native-ble-plx`
- Configured EAS Build for development builds
- **Pivot:** initial plan targeted iPhone via EAS Build, but this required Apple Developer Program enrolment ($99/year) which was not budgeted for this project. Switched target device to an available Samsung Galaxy Tab S9 FE (Android), which requires no developer account for sideloading custom builds
- Reconfigured EAS Build for Android platform and generated a remote Android keystore for app signing

</details>

---

<details>
<summary><strong>Week 5 — React Native UI + BLE Integration (App Side)</strong></summary>

### Slot 6 — Recovery Screen + Clearance Result Screen + BLE Client
**Status:** Done

- Built RecoveryScreen with live progress display
- Built ClearanceResultScreen showing READY / NOT READY
- Integrated `react-native-ble-plx` on the Android development build to scan, connect, and receive BLE notification data

</details>

---

<details>
<summary><strong>Week 6 — AWS Account, IAM, and CLI Setup</strong></summary>

### Slot 7 — AWS Account, IAM, and CLI Setup
**Status:** Done

- Installed AWS CLI v2 and configured authentication
- Created dedicated non-root IAM user `zyntra-dev` with `PowerUserAccess` policy (application development access without IAM administration, billing, or root-level permissions)
- Verified `zyntra-dev` permissions boundary via `aws sts get-caller-identity` and confirmed `AccessDenied` protection on IAM administrative actions
- Configured local AWS CLI profile `zyntra-dev` in region `ap-south-1`

</details>

---

<details>
<summary><strong>Week 7 — Firmware Polish + Perfboard Assembly</strong></summary>

### Slot 8 — Firmware Polish + Perfboard Assembly
**Status:** Upcoming

- Add signal quality check improvements
- Add further RR interval artifact filtering refinement
- Solder all components from breadboard to perfboard for a stable permanent build
- Run passive current draw test to verify 12-hour battery life target

</details>

---

<details>
<summary><strong>Week 8 — State Machine + BLE GATT Server (Firmware Side)</strong></summary>

### Slot 9 — State Machine + BLE GATT Server
**Status:** In Progress

- Implemented six-state machine in code: BASELINE, SHIFT, RECOVERY, CLEARANCE, CLEARED, NOT_CLEARED
- Implemented `config.h` centralising all pin definitions and thresholds
- Implemented `hrv_reset_buffer()` for clean state transitions
- Implemented full BLE GATT server (`ble_service.h/cpp`) with custom Service UUID and Characteristic UUID, supporting both notify (device to phone) and write (phone to device) for hybrid break-trigger logic
- **Not yet completed:** upload and hardware verification of the full state machine on the ESP32; BLE advertisement has not yet been confirmed via nRF Connect

</details>

---

<details>
<summary><strong>Week 9 — PostgreSQL Local Setup + FastAPI REST Backend</strong></summary>

### Slot 10 — PostgreSQL Local Setup + FastAPI REST Backend
**Status:** Upcoming

- Install PostgreSQL via Docker
- Design and create three tables: `workers`, `shift_baselines`, `clearance_events`
- Implement five REST endpoints in FastAPI
- Test all endpoints with curl
- Connect React Native app to local FastAPI

</details>

---

<details>
<summary><strong>Week 10 — RDS Migration</strong></summary>

### Slot 11 — Schema Migration to RDS
**Status:** Upcoming

- Create RDS PostgreSQL instance on free tier (db.t3.micro) under `zyntra-dev` IAM user
- Export local schema using `pg_dump --schema-only` and import to RDS
- Update FastAPI `DATABASE_URL` to RDS endpoint
- Confirm full data flow from React Native through FastAPI into AWS RDS

</details>

---

<details>
<summary><strong>Week 11 — Lambda + API Gateway + SNS</strong></summary>

### Slot 12 — Lambda, API Gateway, and SNS
**Status:** Upcoming

- Add Mangum adapter to make FastAPI Lambda-compatible
- Package and deploy backend to AWS Lambda
- Create API Gateway REST routes pointing to Lambda functions
- Create SNS topic and configure Lambda to trigger on NOT READY events
- Build AuditLogScreen in React Native with full event history and CSV export

</details>

---

<details>
<summary><strong>Week 12 — Sensor Calibration + Validation Begins</strong></summary>

### Slot 13 — Sensor Calibration

- Compare MAX30102 RMSSD against Elite HRV reference app across 10 paired measurements
- Compare MLX90614 readings against a medical infrared thermometer across 10 paired readings
- Document calibration results in `docs/calibration.md`

---

### Slot 14 — Validation Sessions 1 to 5 (Volunteer 1)

- Run treadmill fatigue protocol with Volunteer 1 across 5 sessions on separate days
- Each session: 3-minute baseline rest, 20-minute treadmill at 80% max heart rate, 15-minute monitored break, simultaneous Zyntra clearance test and PVT at break end
- Record device verdict, PVT verdict, and all signal values in CSV

</details>

---

<details>
<summary><strong>Week 13 — Hardening + Report + Demo Video</strong></summary>

### Slot 15 — System Hardening and Battery Test

- Run 12-hour continuous battery life test with all sensors active and BLE advertising
- Assemble final wristband housing
- Perform drop test from 1 metre and water splash test to verify durability
- Run I2C scanner after tests to confirm all sensors still detected

---

### Slot 16 — Project Report and Demo Video

- Record clean 5-minute demonstration video covering the complete system flow
- Write full project report covering problem statement, system design, firmware architecture, mobile app, cloud backend, validation methodology, results, limitations, and future work
- Update GitHub wiki with entries for all completed weeks

</details>

---

<details>
<summary><strong>Week 14 — Final Evaluation</strong></summary>

### Slot 17 — Final Rehearsal

- Run complete end-to-end demonstration three times
- Verify all GitHub commit messages are meaningful and wiki is up to date
- Prepare responses for anticipated Q&A questions

---

### Final Evaluation

**Scope:** Complete Zyntra system demonstrated end to end — ESP32 wristband, React Native supervisor app, AWS cloud backend (API Gateway, Lambda, RDS PostgreSQL, SNS), and validation results from 15 sessions with computed sensitivity and specificity against the PVT gold standard.

</details>
