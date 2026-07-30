# Weekly Project Plan

---

## Overview

| Week | Dates | Focus | Status |
|---|---|---|---|
| Week 3 | Jul 21 - Jul 27 | Hardware verification + HRV + Temperature + OLED firmware | Done |
| Week 4 | Jul 28 - Aug 03 | RT test + Clearance algorithm + State machine + BLE server | In Progress |
| Week 5 | Aug 04 - Aug 10 | Firmware polish + Perfboard + React Native + BLE connection | Upcoming |
| Week 6 | Aug 11 - Aug 17 | React Native screens + Demo preparation + Mid evaluation | Upcoming |
| Week 7 | Aug 18 - Aug 24 | PostgreSQL local setup + FastAPI REST backend | Upcoming |
| Week 8 | Aug 25 - Aug 31 | AWS account setup + RDS PostgreSQL cloud migration | Upcoming |
| Week 9 | Sep 01 - Sep 07 | AWS Lambda + API Gateway + SNS push notifications | Upcoming |
| Week 10 | Sep 08 - Sep 14 | Sensor calibration + Validation sessions 1-5 | Upcoming |
| Week 11 | Sep 15 - Sep 21 | Validation sessions 6-15 + Results analysis | Upcoming |
| Week 12 | Sep 22 - Sep 28 | System hardening + Battery test + Project report + Demo video | Upcoming |
| Week 13 | Sep 29 - Oct 05 | Final rehearsal + Final evaluation | Upcoming |

---

## Weekly Progress

<details>
<summary><strong>Week 3 — Jul 21 to Jul 27 — Hardware Verification + Sensor Firmware</strong></summary>

### Slot 1 — Environment Setup and Hardware Verification
**Status:** Done

- Initialised PlatformIO project with `platformio.ini` configured for ESP32 Dev Module
- Installed all five firmware libraries via `pio lib install`
- Created complete firmware file skeleton across `src/` and `include/` directories
- Wrote and deployed I2C scanner sketch
- Confirmed all three sensors detected at correct addresses: MAX30102 at 0x57, MLX90614 at 0x5A, SSD1306 OLED at 0x3C

---

### Slot 2 — HRV Module + Temperature Module + OLED Display
**Status:** In Progress

- Configure MAX30102 for HRV-optimised mode (400Hz sample rate, 411us pulse width)
- Implement beat detection using `checkForBeat()` to extract RR intervals from PPG signal
- Compute RMSSD from a 2-minute sliding window of RR intervals
- Implement MLX90614 baseline capture by averaging five readings taken 30 seconds apart
- Implement temperature deviation check against the captured baseline
- Build all four OLED screen layouts: shift mode, recovery progress, READY, NOT READY

</details>

---

<details>
<summary><strong>Week 4 — Jul 28 to Aug 03 — RT Test + Clearance Algorithm + BLE</strong></summary>

### Slot 3 — Reaction Time Test + AND-Gate Clearance Logic

- Implement 5-stimulus vibration reaction time test with pseudo-random intervals between 3 and 8 seconds
- Add false start detection — button presses under 100ms from stimulus onset are rejected and the stimulus is repeated
- Add timeout handling — no response within 2000ms is recorded as a failed trial
- Compute median reaction time from 5 valid responses
- Implement AND-gate clearance logic: HRV passes AND temperature passes AND reaction time passes equals CLEARED
- Implement linear regression time-to-clearance estimate displayed on NOT READY screen

---

### Slot 4 — State Machine + BLE GATT Server

- Implement five-state machine: BASELINE, SHIFT, RECOVERY, RT_TEST, CLEARED / NOT_CLEARED
- Implement BLE GATT server with custom Service UUID and Characteristic UUID
- Broadcast JSON payload on every 60-second recovery progress update and on every clearance verdict
- Verify BLE advertisement using nRF Connect app on phone

</details>

---

<details>
<summary><strong>Week 5 — Aug 04 to Aug 10 — Firmware Polish + React Native App</strong></summary>

### Slot 5 — Firmware Polish + Perfboard Assembly

- Add signal quality check — display adjustment prompt on OLED when IR value falls below 50000
- Add RR interval artifact filtering — reject intervals deviating more than 20% from the previous
- Add time-to-clearance estimate on NOT READY screen using recovery curve projection
- Solder all components from breadboard to perfboard for a stable permanent build
- Run passive current draw test to verify 12-hour battery life target

---

### Slot 6 — React Native Project Setup + BLE Connection

- Initialise React Native project
- Install `react-native-ble-plx` and configure Android Bluetooth permissions
- Implement BleService.js — scan, connect, discover GATT services, subscribe to notifications, decode JSON
- Build RecoveryScreen with three animated progress bars updating every 60 seconds from BLE notifications

</details>

---

<details>
<summary><strong>Week 6 — Aug 11 to Aug 17 — Mid Evaluation</strong></summary>

### Slot 7 — Complete App Screens + Demo Preparation

- Build ClearanceResultScreen showing READY or NOT READY with per-signal pass/fail indicators
- Build DashboardScreen showing worker status with colour-coded state
- Run full end-to-end rehearsal at least three times
- Test both READY and NOT READY scenarios on OLED and phone simultaneously
- Record backup demonstration video

---

### Mid Evaluation — 17 August 2026

**Scope:** Complete hardware prototype with firmware and React Native supervisor app connected via BLE. Full clearance protocol demonstrated live — baseline capture, recovery monitoring, reaction time test, and clearance verdict on both OLED and phone. Cloud backend not required at this stage.

</details>

---

<details>
<summary><strong>Week 7 — Aug 18 to Aug 24 — PostgreSQL + FastAPI Backend</strong></summary>

### Slot 8 — Local PostgreSQL Setup

- Install PostgreSQL via Docker
- Design and create three tables: `workers`, `shift_baselines`, `clearance_events`
- Verify table relationships and constraints using psql

---

### Slot 9 — FastAPI REST API

- Implement five REST endpoints: POST /workers, POST /clearance-events, GET /workers/:id/events, GET /events/summary, GET /clearance-events/latest
- Test all endpoints with curl
- Connect React Native ApiService.js to local FastAPI and confirm clearance events save to PostgreSQL

</details>

---

<details>
<summary><strong>Week 8 — Aug 25 to Aug 31 — AWS Setup + RDS PostgreSQL</strong></summary>

### Slot 10 — AWS Account and RDS Instance

- Create AWS account and configure IAM user with programmatic access
- Install and configure AWS CLI
- Create RDS PostgreSQL instance on free tier (db.t3.micro)

---

### Slot 11 — Schema Migration to RDS

- Export local schema using `pg_dump --schema-only`
- Import schema to RDS instance
- Update FastAPI `DATABASE_URL` to RDS endpoint
- Confirm full data flow from React Native through FastAPI into AWS RDS

</details>

---

<details>
<summary><strong>Week 9 — Sep 01 to Sep 07 — Lambda + API Gateway + SNS</strong></summary>

### Slot 12 — AWS Lambda and API Gateway Deployment

- Add Mangum adapter to make FastAPI Lambda-compatible
- Package and deploy backend to AWS Lambda
- Create API Gateway REST routes pointing to Lambda functions
- Update React Native to call the live API Gateway URL

---

### Slot 13 — SNS Push Notifications + Audit Log Screen

- Create SNS topic and configure Lambda to trigger on NOT READY events
- Subscribe supervisor device to SNS topic
- Build AuditLogScreen in React Native with full event history and CSV export

</details>

---

<details>
<summary><strong>Week 10 — Sep 08 to Sep 14 — Sensor Calibration + Validation Begins</strong></summary>

### Slot 14 — Sensor Calibration

- Compare MAX30102 RMSSD against Elite HRV reference app across 10 paired measurements
- Compare MLX90614 readings against a medical infrared thermometer across 10 paired readings
- Document calibration results in `docs/calibration.md`

---

### Slot 15 — Validation Sessions 1 to 5 (Volunteer 1)

- Run treadmill fatigue protocol with Volunteer 1 across 5 sessions on separate days
- Each session: 3-minute baseline rest, 20-minute treadmill at 80% max heart rate, 15-minute monitored break, simultaneous Zyntra clearance test and PVT at break end
- Record device verdict, PVT verdict, and all signal values in CSV

</details>

---

<details>
<summary><strong>Week 11 — Sep 15 to Sep 21 — Complete Validation + Analysis</strong></summary>

### Slot 16 — Validation Sessions 6 to 15 (Volunteers 2 and 3)

- Run identical protocol with Volunteer 2 (5 sessions) and Volunteer 3 (5 sessions)
- Maintain consistent treadmill speed, break duration, and PVT administration across all sessions

---

### Slot 17 — Results Analysis

- Run `compute_metrics.py` on the 15-session CSV dataset
- Compute sensitivity, specificity, accuracy, PPV, and NPV against PVT ground truth
- Generate confusion matrix
- Plot HRV and temperature recovery curves across sessions

</details>

---

<details>
<summary><strong>Week 12 — Sep 22 to Sep 28 — Hardening + Report + Demo Video</strong></summary>

### Slot 18 — System Hardening and Battery Test

- Run 12-hour continuous battery life test with all sensors active and BLE advertising
- Assemble final wristband housing
- Perform drop test from 1 metre and water splash test to verify durability
- Run I2C scanner after tests to confirm all sensors still detected

---

### Slot 19 — Project Report and Demo Video

- Record clean 5-minute demonstration video covering the complete system flow
- Write full project report covering problem statement, system design, firmware architecture, mobile app, cloud backend, validation methodology, results, limitations, and future work
- Update GitHub wiki with entries for all completed weeks

</details>

---

<details>
<summary><strong>Week 13 — Sep 29 to Oct 05 — Final Evaluation</strong></summary>

### Slot 20 — Final Rehearsal

- Run complete end-to-end demonstration three times
- Verify all GitHub commit messages are meaningful and wiki is up to date
- Prepare responses for anticipated Q&A questions

---

### Final Evaluation — 5 October 2026

**Scope:** Complete Zyntra system demonstrated end to end — ESP32 wristband, React Native supervisor app, AWS cloud backend (API Gateway, Lambda, RDS PostgreSQL, SNS), and validation results from 15 sessions with computed sensitivity and specificity against the PVT gold standard.

</details>
