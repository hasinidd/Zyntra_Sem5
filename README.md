# Zyntra -- Post-Break Physiological Readiness Clearance System

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
                   (failed signal shown
                    + time estimate
                    + supervisor alert)
```

### Three Signals

| Signal | Sensor | Metric | Clearance Criterion | Scientific Basis |
|:------|:------|:------|:-------------------|:----------------|
| **Autonomic Recovery** | MAX30102 | RMSSD (HRV) | ≥ 90% of personal baseline | Spring et al. (2018), *Frontiers in Neuroscience* |
| **Thermoregulatory Recovery** | MLX90614 | Wrist skin temperature | Within 0.8°C of baseline | MDPI Sensors validation study (2024) |
| **Cognitive Recovery** | Vibration motor + button | Median reaction time | < 500ms across 5 stimuli | Dinges & Powell (1985) - PVT literature |
---

##  System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                   ESP32 WRISTBAND                       │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────┐   │
│  │ MAX30102 │  │ MLX90614 │  │ Vib Motor│  │ DS3231 │   │
│  │ HRV/SpO2 │  │ IR Temp  │  │ + Button │  │  RTC   │   │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └───┬────┘   │
│       └─────────────┴─────────────┴────────────┘        │
│                        I2C Bus (GPIO 21/22)             │
│  ┌──────────┐  ┌──────────────────────────────────────┐ │
│  │  SSD1306 │  │         ESP32 Main MCU               │ │
│  │   OLED   │  │  Clearance Algorithm + State Machine │ │
│  └──────────┘  │  BLE GATT Server + SPIFFS Logger     │ │
│                └──────────────────┬───────────────────┘ │
│  ┌──────────┐                     │ BLE                 │
│  │ TP4056 + │                     │                     │
│  │3.7V LiPo │                     │                     │
│  └──────────┘                     │                     │
└───────────────────────────────────┼─────────────────────┘
                                    │
                          ┌─────────▼────────────┐
                          │   Flutter Mobile App │
                          │  (Supervisor Phone)  │
                          │  Live recovery dash  │
                          │  Alert notifications │
                          └─────────┬────────────┘
                                    │ Wi-Fi
                          ┌─────────▼─────────────┐
                          │   Firebase Realtime   │
                          │      Database         │
                          │  Clearance audit log  │
                          └───────────────────────┘
```

```mermaid
flowchart TB

%% ===========================
%% Cloud Layer
%% ===========================

subgraph CLOUD["☁️ Cloud Layer — Firebase"]
direction LR

DB["Realtime Database<br/>Clearance Event Log"]
DASH["Supervisor Dashboard<br/>All Workers • Alerts"]
FCM["Push Alerts<br/>FCM Notifications"]

end

%% ===========================
%% Mobile Layer
%% ===========================

subgraph MOBILE["📱 Mobile Layer — Flutter App (Supervisor Phone)"]
direction TB

subgraph ROW1[" "]
direction LR

BLE["BLE Service<br/>Scan • Connect • Parse"]
REC["Recovery Screen<br/>3 Live Progress Bars"]
RESULT["Clearance Result<br/>READY / NOT READY"]

end

subgraph ROW2[" "]
direction LR

AUDIT["Audit Log<br/>History • CSV Export"]
SYNC["Firebase Sync<br/>Wi-Fi Upload"]
ALERT["Alert Manager<br/>Push Notification"]

end

end

%% ===========================
%% Device Layer
%% ===========================

subgraph DEVICE["⌚ Device Layer — ESP32 Wristband (Zyntra)"]
direction TB

subgraph SENSORS["Sensors"]
direction LR

MAX["MAX30102<br/>PPG → HRV (RMSSD)"]
MLX["MLX90614<br/>IR Skin Temperature"]
BUTTON["Motor + Button<br/>Reaction Test"]

end

ALG["ESP32 Clearance Algorithm<br/><br/>HRV ≥ 90% Baseline<br/>Temp ≤ 0.8°C<br/>RT < 500 ms"]

subgraph OUTPUT["Outputs"]
direction LR

OLED["SSD1306 OLED<br/>READY / NOT READY"]
BLESERVER["BLE GATT Server<br/>JSON Broadcast"]
SPIFFS["SPIFFS Flash Log<br/>100 Offline Events"]

end

MAX --> ALG
MLX --> ALG
BUTTON --> ALG

ALG --> OLED
ALG --> BLESERVER
ALG --> SPIFFS

end

%% ===========================
%% Cross-layer Communication
%% ===========================

BLESERVER -- "BLE Notify" --> BLE

SYNC -- "Wi-Fi / HTTPS" --> DB
SYNC --> DASH
SYNC --> FCM

%% ===========================
%% Styling
%% ===========================

style CLOUD fill:#14532d,color:#fff,stroke:#22c55e,stroke-width:2px
style MOBILE fill:#1e3a8a,color:#fff,stroke:#60a5fa,stroke-width:2px
style DEVICE fill:#7c2d12,color:#fff,stroke:#fb923c,stroke-width:2px

style ALG fill:#0f172a,color:#fff
style DB fill:#111827,color:#fff
style DASH fill:#111827,color:#fff
style FCM fill:#111827,color:#fff
style BLE fill:#111827,color:#fff
style REC fill:#111827,color:#fff
style RESULT fill:#111827,color:#fff
style AUDIT fill:#111827,color:#fff
style SYNC fill:#111827,color:#fff
style ALERT fill:#111827,color:#fff
style MAX fill:#111827,color:#fff
style MLX fill:#111827,color:#fff
style BUTTON fill:#111827,color:#fff
style OLED fill:#111827,color:#fff
style BLESERVER fill:#111827,color:#fff
style SPIFFS fill:#111827,color:#fff
```
