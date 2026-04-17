# Safi V2 Pasteurizer — Firmware Project Plan

**Project:** Safi V2 Production Firmware
**Target completion:** 2026-05-15 (functional prototype)
**Code repository:** github.com/filipbirger/Safi_V2_Test_App

---

## What Is This Device?

The Safi V2 is a portable milk pasteurizer. It heats milk to a safe temperature,
holds it there for a set amount of time, then cools it — a process called HTST
(High-Temperature Short-Time) pasteurization. The firmware is the software running
on the device that controls every part of this process: the motor, the display,
the buttons, the battery, and the connection to the cloud.

---

## How the Device Works — Plain Language Overview

### Inputs (things the device receives or detects)
| Input | How it works |
|---|---|
| Power button | Wakes the device from standby (2.8 µA sleep current when off) |
| Button 1 / 2 / 3 | Navigate menus and start/stop pasteurization |
| Temperature sensor | Probes the milk temperature every 750 ms via a waterproof external sensor |
| Battery level | Monitored continuously via a dedicated chip on the I2C bus |
| USB charging (magnetic connector) | Detected automatically; charger IC manages the 2-cell battery |
| Motor current | Measured by an op-amp circuit 1000+ times per second to detect a jam or overload |

### Outputs (things the device does or shows)
| Output | How it works |
|---|---|
| LCD screen (142×428 px) | Shows current temperature, battery %, device state, and button options |
| Backlight | PWM-dimmed; off during sleep to save power |
| Motor | Variable-speed pump driven by PWM; runs during PREHEAT, PASTEURIZE, and COOL |
| Buzzer | Beeps at 2.7 kHz to signal cycle start, completion, and faults |
| RGB LED | Changes colour to indicate device status (idle, active, fault, charging) |
| Cloud (Particle LTE) | Uploads a pasteurization record after every completed cycle |

---

## Pasteurization Cycle — Step by Step

### Normal cycle
| Step | What triggers it | What the device does | Time / condition to advance |
|---|---|---|---|
| **IDLE** | Power-on or cycle complete | Shows home screen (temp, battery, status) | User presses START |
| **PREHEATING** | START pressed | Motor runs, screen shows live temp and progress | Until milk reaches **74°C** |
| **PASTEURIZING** | Milk reaches 74°C | Motor continues, countdown timer starts, screen shows countdown | **15 seconds** at ≥ 74°C continuously |
| **COOLING** | 15-second hold complete |  Motor continues to circulate, screen shows cooling progress | Until milk drops to a safe temperature |
| **COMPLETE** | Safe cool temperature reached | Buzzer sounds, green LED, record uploaded to cloud, screen shows summary | User presses OK or 30-second auto-timeout |
| **IDLE** | User confirms or timeout | Returns to home screen ready for next cycle | — |

### Fault / abort at any step
| Fault condition | What triggers it | What happens immediately |
|---|---|---|
| Motor overload / jam | Current exceeds 2.05 A | Motor stops instantly, FAULT state, red LED, buzzer alarm |
| Temperature sensor disconnected | No valid reading for > 2 seconds | FAULT state, cycle aborted, error shown on screen |
| Cycle timeout | Step takes longer than expected (e.g. milk not heating) | FAULT state, watchdog triggers |
| Low battery | Battery monitor reports critically low charge | Warning shown; cycle will not start; buzzer chirp |

---

## Timing Constraints

These are the time limits the firmware must meet to operate correctly and safely.

| Constraint | Value | Why it matters |
|---|---|---|
| Main control loop cycle | < 10 ms | Temperature and safety checks run every loop; longer loops mean slower fault response |
| Temperature sample interval | 750 ms | DS18B20 sensor conversion time; firmware reads a new value every 750 ms |
| Safety check interval | Every loop (< 10 ms) | Overcurrent and sensor-disconnect checks must be near-instant |
| Screen partial update (temp / battery) | < 1 ms | Keeps the loop fast; only small regions of the screen are redrawn |
| Screen full image swap (state change) | ~60 ms | Only allowed during state transitions, not mid-cycle |
| Button debounce window | ~20 ms | Prevents a single press registering as multiple presses |
| Cloud upload retry backoff | Exponential (first retry 1 s, doubles each attempt) | Prevents flooding the network if offline |
| Offline data storage capacity | ~48 hours (~2.5 KB/day at 20 cycles/day) | Records are stored in flash if cloud is unavailable |

---

## Data Recorded Per Cycle

Every completed pasteurization cycle produces one record that is stored locally
and uploaded to the cloud when connected.

| Field | Description |
|---|---|
| Timestamp | Date and time the cycle started |
| Start temperature | Milk temperature at cycle start |
| Peak temperature | Highest temperature reached during PASTEURIZE |
| Hold time achieved | Actual seconds held at ≥ 74°C (must be ≥ 15 s for a valid cycle) |
| End temperature | Temperature when COMPLETE was triggered |
| Cycle result | PASS or FAULT |
| Battery % at start | State of charge when cycle began |

---

## Battery & Power

| Detail | Value |
|---|---|
| Battery type | 2-cell lithium-ion (2S), 8.4 V fully charged |
| Charging | Via magnetic USB connector, managed automatically at up to 1.7 A |
| Standby current | 2.8 µA (device appears off; power button wakes it) |
| Charging states | Not charging → Pre-charge → Fast charge → Complete |
| Fault protection | Overvoltage, overcurrent, thermal shutdown, NTC temperature monitoring |

---

## Cloud Connectivity

The device uses an LTE modem (Particle B524) to send data to the cloud.

- Records are queued locally in flash memory and uploaded when connected.
- If the upload fails, the device retries with increasing wait times (1 s, 2 s, 4 s…).
- If the queue fills (> 48 hours of offline cycles), the oldest record is overwritten.
- Once a record is confirmed received by the cloud, it is removed from local storage.
- Remote firmware updates are supported over the air (OTA) via Particle Cloud.

---

## What Each Button Does

| Button | Short press | Long press |
|---|---|---|
| Power (BTN_PWR) | Wake from sleep | — |
| Button 1 | Navigate / confirm | Start cycle (from home screen) |
| Button 2 | Navigate / back | — |
| Button 3 | Navigate / cancel | Abort cycle |

*Exact button assignments will be finalised during UI development (stories #18–21).*

---

## Status Indicators

### RGB LED colours
| Colour | Meaning |
|---|---|
| White (dim) | Idle / standby |
| Green | Pasteurization complete successfully |
| Blue (pulsing) | Preheating or cooling in progress |
| Yellow | Low battery warning |
| Red (flashing) | Fault — device stopped, action required |
| Cyan | Charging |

### Buzzer patterns
| Pattern | Meaning |
|---|---|
| Single short beep | Button press confirmed |
| Three ascending beeps | Cycle started |
| Long double beep | Cycle complete |
| Rapid repeating beeps | Fault alarm |

---

## Work Breakdown — Story List

Stories are grouped by area. Each story is one unit of work for one developer.

### Foundation
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 1  | Create main program skeleton with setup and repeating task loop               |          |        |
| 2  | Define hardware pin map — resolve all wiring conflicts between components     | Filip    | Review |
| 3  | Implement power-on sequence (correct order to turn on each subsystem)         |          |        |
| 4  | Implement sleep/wake (press power button to wake from 2.8 µA standby)        |          |        |
| 5  | Set up GitHub branch and pull-request workflow for the team                   |          |        |

### Driver Fixes (existing code that needs correction)
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 6  | Fix motor driver: wrong file path, add motor power enable, add temperature read |       |        |
| 7  | Fix battery charger driver: add fault reading, fix function signatures        |          |        |
| 8  | Fix temperature sensor driver: wrong pin used (must be P0.07)                 |          |        |
| 9  | Add error return values to buzzer driver                                      |          |        |
| 10 | Merge all corrected drivers into shared development branch                    |          |        |

### New Drivers (components not yet written)
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 11 | LCD display driver: initialise screen, draw text, shapes, images, backlight   | Filip    | Review |
| 12 | Button driver: debounce all 4 buttons, detect short and long press            | Dev B    |        |
| 13 | RGB LED driver: set colour, run blink patterns, define status modes           | Dev B    |        |

### Pasteurization Control
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 14 | HTST state machine: IDLE → PREHEAT → PASTEURIZE → COOL → COMPLETE → FAULT    |          |        |
| 15 | Motor control: speed ramp profiles, direction, stop on overcurrent            | Dev C    |        |
| 16 | Temperature monitoring loop: non-blocking reads, moving average, fault detect | Dev B    |        |
| 17 | Safety interlocks: overcurrent shutdown, sensor disconnect, timeout watchdog  |          |        |

### User Interface
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 18 | UI framework: screen manager, menu navigation, button event routing           |          |        |
| 19 | Home screen: battery %, current temperature, device status                    |          |        |
| 20 | Active cycle screen: progress bar, countdown timer, live temperature          |          |        |
| 21 | Settings / info screen                                                        |          |        |
| 22 | Buzzer and RGB LED feedback patterns for start, complete, and fault           |          |        |

### Cloud & Data Storage
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 23 | Flash storage queue: persistent circular buffer with error-checking, power-safe |        |        |
| 24 | Queue manager: add records on events, upload oldest first when connected      |          |        |
| 25 | Queue overflow handling: overwrite oldest record, alert when queue is full    |          |        |
| 26 | Particle Cloud integration: publish records, expose device variables          |          |        |
| 27 | Pasteurization record format: all fields listed in "Data Recorded" section above |       |        |
| 28 | Cloud sync service: monitor connection, auto-upload, retry with backoff       |          |        |
| 29 | Over-the-air firmware update: validate Particle built-in OTA works correctly  |          |        |

### Event Manager & Task Scheduling
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 34 | Define all event types, priorities, and data payloads                         |          |        |
| 35 | Implement priority event queue: 3 tiers, safe to post from interrupts         |          |        |
| 36 | Implement event dispatch: register handlers, fire callbacks in priority order |          |        |
| 37 | Define synchronous task loop: temperature + safety checks run every iteration |          |        |
| 38 | Connect button driver to event manager: post SHORT/LONG press events          |          |        |
| 39 | Connect temperature and motor to event manager: post fault events             |          |        |
| 40 | Connect battery and charger to event manager: post low battery, fault, complete |        |        |
| 41 | Connect state machine to event manager: broadcast state changes to all listeners |       |        |
| 42 | Connect cloud publish to event manager: cycle complete triggers upload        |          |        |

### Integration & Testing
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 30 | Full integration of all subsystems into main loop                             |          |        |
| 31 | Hardware-in-loop testing on real PCB                                          |          |        |
| 32 | Edge case testing: power loss mid-cycle, sensor disconnect, low battery, full queue |   |        |
| 33 | Power consumption profiling and sleep mode optimisation                       |          |        |

---

## Week-by-Week Schedule

| Week | Dev A — Drivers & Motor        | Dev B — UI & Battery             | Dev C — App & Cloud                |
|------|-------------------------------|----------------------------------|-------------------------------------|
| 1    | #6 Fix motor, #10 Merge       | #7 Fix charger, #8 Fix temp pin  | #1 Main skeleton, #2 Pin map, #5 GH |
| 2    | #11 LCD driver                | #12 Buttons, #13 RGB, #9 Buzzer  | #3–4 Power management, #14 State machine |
| 3    | #18–20 UI screens             | #15 Motor integration, #16 Temp loop | #17 Safety, #23–25 Flash queue  |
| 4    | #22 Feedback patterns, #30    | #31 Hardware test, #32 Edge cases | #26–29 Cloud + OTA, #33 Power profile |

*Event manager stories #34–42 are distributed across weeks 2–3 alongside the above.*

---

## Technical Reference — Hardware Components

| Component | Part number | How it connects | Pins / Address |
|---|---|---|---|
| Motor driver | TB67H450AFNG | PWM + GPIO | MOTPWM1=P0.12, MOTPWM2=P0.24, VMOTEN=P0.13 |
| Motor current sense | MCP6006T op-amp + 200 mΩ shunt | ADC | IM=ADC2, trips at 2.05 A |
| Temperature sensor | DS18B20 (external probe) | 1-Wire | P0.07 |
| LCD display | ER-TFT2.79-1 (NV3007 controller) | SPI | CS=P1.08, DC=P1.09, RST=P0.11 |
| LCD backlight | BSS138PW MOSFET | PWM | P1.01 |
| Buzzer | CMI-1295-03TH | PWM | P1.04 (2.7 kHz resonant) |
| Buttons (×4) | Membrane keyboard | GPIO | BTN_PWR, BTN_1=P0.31, BTN_2=P0.05, BTN_3=TBD |
| RGB LED | Membrane keyboard | PWM | R=P0.16, G=P0.15, B=P0.14 |
| Battery charger | MP2672AGD | I2C (addr 0x4B) | 2-cell 8.4 V, up to 1.7 A |
| Battery monitor | PAC1951T-1E/4MX | I2C (addr 0x10) | ALERT1=P1.03, ALERT2=ADC1, PWRDN=P0.30 |
| Power switch controller | XC6192AA10ER-G | GPIO | PWR_SHDN=P0.02, SWOUT=P0.28 |
| Debug header | J502 | UART | TX=P0.06, RX=P0.08 |
| USB charging port | Magnetic connector | USB | — |

**Power rails:** 3.3 V standby (always on, 2.8 µA) · 4 V SoM · 3.3 V peripherals · 6 V / 3.5 A motor
**Processor module:** Particle B524MEA (Nordic nRF52840 + Quectel LTE modem)

---

## Technical Reference — Driver Status

| # | Driver | Code branch | Quality | Known issues |
|---|---|---|---|---|
| — | DS18B20 temperature sensor | feature/temp_sensor_driver | 5/5 | Wrong pin (D2 used, must be P0.07) — fix in story #8 |
| — | PAC1951T battery monitor | feature/battery_charger_driver | 5/5 | Minor: hardcoded energy constant |
| — | Buzzer PWM | feature/Buzzer_PWM_Driver | 4/5 | No error return codes, no off-before-init guard |
| — | TB67H450AFNG motor driver | feature/motor-controlller-driver | 4/5 | Wrong include path, missing Power_SetMotorSupply() |
| — | MP2672AGD charger | feature/battery_monitor_driver | 3.5/5 | GetFaults() not implemented, signature mismatch, circular read/write |

---

## Technical Reference — Event Manager

### What is the event manager?
It is a messaging system inside the firmware. When something happens (a button is
pressed, the temperature changes, a fault occurs), that event is posted to a
priority queue. Other parts of the firmware register to be notified when specific
events occur and are called automatically when those events are dispatched.

### Why priority levels?
Not all events are equally urgent. A motor overcurrent must be acted on immediately.
A cloud upload can wait. The three tiers ensure the most critical things are always
handled first regardless of how busy the system is.

### Priority tiers
| Priority | Examples | Acceptable delay |
|---|---|---|
| CRITICAL | Temperature fault, motor overcurrent, sensor disconnect, watchdog timeout | < 10 ms (handled before anything else) |
| NORMAL | State change, pasteurization timer done, button press | < 20 ms |
| LOW | Low battery warning, charge complete, cloud connected, LED pattern change | Up to 500 ms |

### What runs outside the event queue (safety-critical path)
Temperature sampling and safety checks run on every single loop iteration —
they are not queued. This guarantees they can never be delayed by a backlog of
other events.

    Every ~10 ms:
      1. Check if a new temperature reading is ready → latch it
      2. Evaluate all safety conditions right now → stop motor immediately if fault
      3. Advance the state machine based on current readings
      4. Drain the event queue (CRITICAL first, then NORMAL, then LOW)

### Event data structure (for developers)
    typedef struct {
        EventType_t     type;      // what happened
        EventPriority_t priority;  // CRITICAL / NORMAL / LOW
        union {
            uint8_t  buttonId;     // which button was pressed
            float    tempC;        // temperature value at time of event
            uint8_t  stateId;      // which state was entered
            uint16_t faultMask;    // bitmask of active faults
        } data;
    } Event_t;

---

## Technical Reference — Flash Storage Queue

- Circular buffer stored in non-volatile flash memory
- Each record has a CRC-16 checksum — corrupt records are detected and skipped
- A record is only deleted after the cloud confirms it was received
- If the device loses power mid-write, the partial record is detected and discarded
- Queue capacity: approximately 48 hours of offline operation at 20 cycles/day (~2.5 KB/day)
- When full: oldest record is overwritten (same policy as a CCTV loop recording)

---

## Key Design Decisions

| Decision | Choice | Reason |
|---|---|---|
| Firmware platform | Particle DeviceOS | LTE + OTA built in; faster time to prototype |
| Pasteurization standard | HTST: 74°C for 15 seconds | Industry standard for milk safety |
| Cloud provider | Particle Cloud | Native to the hardware module |
| Queue overflow policy | Overwrite oldest | Preserves most recent data; acceptable for this use case |
| Scheduling model | Cooperative (no RTOS) | Sufficient for the task count; simpler to debug |
| Safety path | Synchronous (not queued) | Guarantees fault response time regardless of queue load |
| Story tracking | Single Jira epic, 42 stories | — |
| Branch strategy | feature → develop → main | Standard; reviewed before merge |
| Regulatory certification | Not required at this stage | Prototype only |
