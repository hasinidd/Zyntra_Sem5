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


##  System Architecture

mermaid
flowchart TB

subgraph Cloud["Cloud Layer - Firebase"]
    direction LR
    DB["Realtime Database<br/>Clearance Event Log"]
    DASH["Supervisor Dashboard<br/>All Workers & Alerts"]
    FCM["Push Alerts<br/>FCM Notifications"]
end

subgraph Mobile["Mobile Layer - Flutter App (Supervisor Phone)"]
    direction TB

    subgraph MobileTop[" "]
        direction LR
        BLE["BLE Service<br/>Scan, Connect, Parse"]
        REC["Recovery Screen<br/>Live Progress Bars"]
        RESULT["Clearance Result<br/>READY / NOT READY"]
    end

    subgraph MobileBottom[" "]
        direction LR
        AUDIT["Audit Log<br/>History, CSV Export"]
        SYNC["Firebase Sync<br/>Wi-Fi Upload"]
        ALERT["Alert Manager<br/>Push Notification"]
    end
end

subgraph Device["Device Layer - ESP32 Wristband"]
    direction TB

    subgraph Sensors["Sensors"]
        direction LR
        MAX["MAX30102<br/>PPG → HRV (RMSSD)"]
        TEMP["MLX90614<br/>Skin Temperature"]
        RT["Motor + Button<br/>Reaction Test"]
    end

    ALG["Clearance Algorithm<br/>HRV ≥ 90% Baseline<br/>Temperature ≤ 0.8°C<br/>Reaction Time < 500 ms"]

    subgraph Outputs["Outputs"]
        direction LR
        OLED["SSD1306 OLED<br/>READY / NOT READY"]
        GATT["BLE GATT Server<br/>JSON Broadcast"]
        LOG["SPIFFS Flash Log<br/>100 Offline Events"]
    end

    MAX --> ALG
    TEMP --> ALG
    RT --> ALG

    ALG --> OLED
    ALG --> GATT
    ALG --> LOG
end

GATT -- BLE Notify --> BLE
SYNC -- Wi-Fi / HTTPS --> DB
SYNC --> DASH
SYNC --> FCM

