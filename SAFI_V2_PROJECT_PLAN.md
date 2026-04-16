# Safi V2 Pasteurizer — Firmware Project Plan

**Epic:** Safi V2 Production Firmware
**Timeline:** 1 month (~2026-05-15), functional prototype
**Team:** Filip (lead/reviewer) + 3 junior developers
**Platform:** Particle DeviceOS on B524MEA (nRF52840 + LTE)
**Repo:** github.com/filipbirger/Safi_V2_Test_App

---

## Hardware Summary

| Subsystem       | IC                  | Interface  | Pin(s)                                          |
|-----------------|---------------------|------------|-------------------------------------------------|
| Motor driver    | TB67H450AFNG        | PWM+GPIO   | MOTPWM1=P0.12, MOTPWM2=P0.24, VMOTEN=P0.13     |
| Motor current   | MCP6006T + 200mR    | ADC        | IM=ADC2, trip at 2.05A                           |
| Temp sensor     | DS18B20 (external)  | 1-Wire     | 1WIRE=P0.07                                      |
| LCD display     | ER-TFT2.79-1        | SPI        | CS=P1.08, RS=P1.09, RSTB=P0.11                  |
| LCD backlight   | BSS138PW MOSFET     | PWM        | LCD_BL=P1.01                                     |
| Buzzer          | CMI-1295-03TH       | PWM        | BZR=P1.04 (2.7kHz)                              |
| Buttons (x4)    | Membrane keyboard   | GPIO       | BTN_PWR, BTN_1=P0.31, BTN_2=P0.05, BTN_3        |
| RGB LED         | On membrane keyboard| PWM        | R=P0.16, G=P0.15, B=P0.14                       |
| Battery charger | MP2672AGD           | I2C        | 2-cell 8.4V, 1.7A CC                            |
| Current monitor | PAC1951T-1E/4MX     | I2C(SMBus) | Addr 0x4B, ALERT1=P1.03, ALERT2=ADC1, PWRDN=P0.30 |
| Power switch    | XC6192AA10ER-G      | GPIO       | PWR_SHDN=P0.02, SWOUT=P0.28 (2.8uA standby)    |
| Debug UART      | J502 header         | UART       | Tx, Rx                                           |
| USB             | Magnetic connector  | USB        | USBD+, USBD-                                    |

**Power rails:** 3.3V_STBY (always-on, 2.8uA) | 4V (SoM) | 3.3V (peripherals) | 6V/3.5A (motor)
**Battery:** 2S Li-ion, 8.4V, charged via magnetic USB
**Motor:** No-load 0.1A, Rated 0.45A, Stall 2A

---

## Existing Driver Status

| # | Driver              | Branch                            | Quality | Issues                                              |
|---|---------------------|-----------------------------------|---------|-----------------------------------------------------|
| - | DS18B20 temp sensor | feature/temp_sensor_driver        | 5/5     | Pin defined as D2, must be P0.07                    |
| - | PAC1951T batt mon   | feature/battery_charger_driver    | 5/5     | Minor: hardcoded energy constant                    |
| - | Buzzer PWM          | feature/Buzzer_PWM_Driver         | 4/5     | No error returns, no off-before-init                |
| - | TB67H450AFNG motor  | feature/motor-controlller-driver  | 4/5     | Wrong include path, missing Power_SetMotorSupply()  |
| - | MP2672AGD charger   | feature/battery_monitor_driver    | 3.5/5   | GetFaults() missing, signature mismatch, circular R/W |

---

## Story List

### Foundation
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 1  | Create main .ino skeleton with setup()/loop() and task scheduling             |          |        |
| 2  | Define board pin map header — resolve all pin conflicts                       |          |        |
| 3  | Implement power-on sequencing (rail enable order, peripheral init)            |          |        |
| 4  | Implement power management (sleep/wake via XC6192 button controller)          |          |        |
| 5  | Set up GitHub branch strategy (develop branch, PR templates)                  |          |        |

### Driver Fixes
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 6  | Fix motor driver: include path, add Power_SetMotorSupply(), Board_GetTemperatureC() |     |        |
| 7  | Fix MP2672AGD: implement GetFaults(), fix signature mismatch, remove circular R/W |      |        |
| 8  | Fix DS18B20 pin assignment to match hardware (P0.07)                          |          |        |
| 9  | Add error return codes to buzzer driver                                       |          |        |
| 10 | Merge all fixed drivers into develop with unified folder structure            |          |        |

### New Drivers
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 11 | LCD display driver (ER-TFT2.79-1 SPI — init, clear, text, shapes, backlight) |          |        |
| 12 | Button input handler (4-button debounce, short/long press detection)          |          |        |
| 13 | RGB LED driver (color set, blink patterns, status modes)                      |          |        |

### Pasteurization Control
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 14 | HTST state machine (IDLE > PREHEAT > PASTEURIZE > COOL > COMPLETE > FAULT)   |          |        |
| 15 | Motor control integration (ramp profiles, direction, overcurrent abort)       |          |        |
| 16 | Temperature monitoring loop (non-blocking DS18B20, moving avg, fault detect)  |          |        |
| 17 | Safety interlocks (overcurrent shutdown, sensor disconnect, timeout watchdog) |          |        |

### User Interface
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 18 | UI framework (screen manager, menu navigation, button event routing)          |          |        |
| 19 | Home screen (battery %, temperature, device status)                           |          |        |
| 20 | Pasteurization active screen (progress bar, countdown, live temp)             |          |        |
| 21 | Settings / info screen (placeholder for display mockups)                      |          |        |
| 22 | Buzzer + RGB LED feedback patterns (start, complete, fault)                   |          |        |

### Message Queue & Cloud
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 23 | Persistent FIFO message queue in flash (circular buffer, CRC-16, power-safe)  |          |        |
| 24 | Queue manager: enqueue on events, drain oldest-first when connected           |          |        |
| 25 | Queue overflow: overwrite oldest record, queue-full alert                     |          |        |
| 26 | Particle Cloud integration (functions, variables, publish via FIFO)           |          |        |
| 27 | Pasteurization record schema (timestamp, temps, duration, result, battery %)  |          |        |
| 28 | Cloud sync service (connection monitor, auto-drain, retry with backoff)       |          |        |
| 29 | OTA firmware update support (validate Particle built-in OTA)                  |          |        |

### Integration & Testing
| #  | Story                                                                         | Assignee | Status |
|----|-------------------------------------------------------------------------------|----------|--------|
| 30 | Full integration of all subsystems into main loop                             |          |        |
| 31 | Hardware-in-loop testing on real PCB                                          |          |        |
| 32 | Edge case testing (power loss, sensor disconnect, low battery, queue full)    |          |        |
| 33 | Power consumption profiling and sleep mode optimization                       |          |        |

---

## Week-by-Week Schedule

| Week | Dev A (Drivers/Motor)          | Dev B (UI/Battery)               | Dev C (App/Cloud)                  |
|------|-------------------------------|----------------------------------|-------------------------------------|
| 1    | #6 Fix motor, #10 Merge      | #7 Fix MP2672, #8 Fix DS18B20   | #1 Main skeleton, #2 Pin map, #5 GH |
| 2    | #11 LCD display driver        | #12 Buttons, #13 RGB, #9 Buzzer | #3-4 Power mgmt, #14 State machine  |
| 3    | #18-20 UI screens             | #15 Motor integ, #16 Temp loop  | #17 Safety, #23-25 FIFO queue       |
| 4    | #22 Feedback patterns, #30    | #31 HW test, #32 Edge cases     | #26-29 Cloud+OTA, #33 Power profile |

---

## FIFO Message Queue Design

- Persistent circular buffer in flash, CRC-16 per message
- Only pop after Particle.publish() ACK — no data loss during normal operation
- Overflow policy: overwrite oldest record when full
- Sized for ~48hrs offline (~2.5KB/day at 20 cycles)
- Head/tail pointers persisted to survive power cycles
- Cloud sync drains oldest-first on reconnect with exponential backoff

---

## Pasteurization State Machine

    IDLE --[BTN_START]--> PREHEATING
    PREHEATING --[temp >= 72C]--> PASTEURIZING (15s timer, motor ON)
    PASTEURIZING --[timer done]--> COOLING (motor ON, heater OFF)
    COOLING --[temp < safe]--> COMPLETE (buzzer, log record, enqueue to FIFO)
    ANY STATE --[fault]--> FAULT (overcurrent, sensor fail, timeout)
    COMPLETE/FAULT --[BTN or timeout]--> IDLE

---

## Key Design Decisions

- Platform: Particle DeviceOS (not Zephyr)
- HTST parameters: 72C for 15 seconds (standard)
- Cloud: Particle Cloud (publish/subscribe/functions)
- FIFO overflow: overwrite oldest
- Single Jira epic for all 33 stories
- GitHub: feature branches -> develop -> main
- No regulatory certification requirements at this stage
