# Zyntra - Post-Break Physiological Readiness Clearance System

**CS3283 - Embedded Systems Project | Semester 5 | University of Moratuwa**

**Zyntra** is an ESP32-based wristband that checks three body signals during a rest break and gives a reliable **READY / NOT READY** result before a worker goes back to a dangerous job. It helps replace guessing with accurate data.

##  The Problem

In dangerous jobs such as crane operation, scaffolding, electrical work, and heavy machinery operation, a small mistake caused by tiredness or poor physical condition can lead to serious injuries or even death.

Workers take required rest breaks after heavy work, heat exposure, near-miss accidents, or machine vibration. After the break, they return to their tasks. However, the important question is: *"Has this worker recovered enough and is ready to safely continue the job?"*

Currently this is answered by:
- **Fixed-time schedules** - 10 minutes for everyone, regardless of individual physiology
- **Self-assessment** - "I feel fine" : which is systematically unreliable


---

##  The Solution

A wrist-worn ESP32 device that monitors **three body recovery signals** during a rest break. It gives a **READY** clearance only when all three signals confirm that the worker has recovered, using an AND-gate decision logic.

```
          Break Starts
               │
               ▼
┌─────────────────────────────┐
│        CLEARANCE LOGIC      │
│  ┌─────┐  ┌──────┐  ┌────┐  │
│  │ HRV │  │ TEMP │  │ RT │  │  ← ALL THREE MUST PASS
│  └──┬──┘  └──┬───┘  └─┬──┘  │
└─────┼────────┼────────┼─────┘
      └────────┴────────┘
               │
     ┌─────────┴────────┐
     ▼                  ▼
  READY ✅         NOT READY ❌
  (OLED green)     (failed signal shown
                    + time estimate
                    + supervisor alert)
```

### Three Signals

| Signal | Sensor | Metric | Clearance Criterion | Scientific Basis |
|:------|:------|:------|:-------------------|:----------------|
| **Autonomic Recovery** | MAX30102 | RMSSD (HRV) | ≥ 90% of personal baseline | Spring et al. (2018), *Frontiers in Neuroscience* |
| **Thermoregulatory Recovery** | MLX90614 | Wrist skin temperature | Within 0.8°C of baseline | MDPI Sensors validation study (2024) |
| **Cognitive Recovery** | Vibration motor + button | Median reaction time | < 500ms across 5 stimuli | Dinges & Powell (1985) — PVT literature |
---

##  System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                   ESP32 WRISTBAND                        │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────┐ │
│  │ MAX30102 │  │MLX90614  │  │ Vib Motor│  │DS3231  │ │
│  │ HRV/SpO2 │  │ IR Temp  │  │ + Button │  │  RTC   │ │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └───┬────┘ │
│       └─────────────┴──────────────┴─────────────┘      │
│                        I2C Bus (GPIO 21/22)               │
│  ┌──────────┐  ┌──────────────────────────────────────┐ │
│  │SSD1306   │  │         ESP32 Main MCU               │ │
│  │   OLED   │  │  Clearance Algorithm + State Machine │ │
│  └──────────┘  │  BLE GATT Server + SPIFFS Logger     │ │
│                └───────────────────┬──────────────────┘ │
│  ┌──────────┐                      │ BLE                 │
│  │TP4056 +  │                      │                     │
│  │3.7V LiPo │                      │                     │
│  └──────────┘                      │                     │
└───────────────────────────────────┼─────────────────────┘
                                    │
                          ┌─────────▼──────────┐
                          │   Flutter Mobile App │
                          │  (Supervisor Phone)  │
                          │  Live recovery dash  │
                          │  Alert notifications │
                          └─────────┬────────────┘
                                    │ Wi-Fi
                          ┌─────────▼────────────┐
                          │   Firebase Realtime   │
                          │      Database         │
                          │  Clearance audit log  │
                          └──────────────────────┘
```

##  Project Structure

```


