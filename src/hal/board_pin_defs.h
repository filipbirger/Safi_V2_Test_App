#pragma once

// =============================================================================
// board_pin_defs.h — canonical GPIO / I2C address map for SAFI v2
// Platform : Particle DeviceOS, B524MEA (nRF52840 + LTE)
// Reference: Hardware schematic + Particle B-Series SoM datasheet
//
// HOW TO USE
//   #include "board_pin_defs.h"
//   in every driver and application file that needs a pin or I2C address.
//   Remove all driver-local pin #defines — they belong here only.
//
// PARTICLE B524 ALIAS CONVENTION
//   Particle alias → nRF52840 GPIO (confirmed from B-Series SoM datasheet)
//   D0  = P0.26    D1  = P0.27    D2  = P1.01    D3  = P1.02
//   D4  = P1.08    D5  = P1.10    D6  = P1.11    D7  = P1.12
//   D8  = P1.03    D9  = P0.09
//   A0  = P0.04    A1  = P0.05    A2  = P0.28    A3  = P0.29
//   A4  = P0.30    A5  = P0.31
//   SCK = P1.15    MOSI = P1.13   MISO = P1.14
//   TX  = P0.06    RX  = P0.08
//   Pins marked VERIFY must be confirmed against the full B524 SoM pad table.
//
// CONFLICTS RESOLVED (vs. driver branches as of 2026-04-16)
//   DS18B20 driver    : DS18B20_PIN = D2  (P1.01) → WRONG, schematic = P0.07
//   Motor driver      : MOTPWM1    = D2  (P1.01) → WRONG, schematic = P0.12
//   Motor driver      : MOTPWM2    = D3  (P1.02) → WRONG, schematic = P0.24
//   Buzzer driver     : BUZZER_PIN = D6  (P1.11) → needs verify, schematic = P1.04
//   LCD driver        : LCD_PIN_RST= D6  (P1.11) → WRONG, schematic = P0.11
//   LCD driver        : LCD_PIN_CS = D8  (P1.03) → conflicts with PAC ALERT1
//   PAC1951T plan     : I2C addr listed as 0x4B  → WRONG, that is MP2672AGD's addr
//                       PAC1951T with ADDR=GND = 0x10 (confirmed in driver)
//
// OPEN ITEMS (need schematic clarification)
//   BTN_PWR pin number not listed in project plan
//   BTN_3   pin number not listed in project plan
// =============================================================================


// -----------------------------------------------------------------------------
// Motor Driver — TB67H450AFNG
// Schematic: MOTPWM1=P0.12, MOTPWM2=P0.24, VMOTEN=P0.13
// VERIFY: P0.12, P0.13, P0.24 do not map to standard D/A aliases on B524.
//         Check B524 SoM pad table for the correct Particle constant.
// pins_init() skips these until aliases are confirmed.
// -----------------------------------------------------------------------------
// #define PIN_MOTPWM1     // TODO: P0.12 — confirm Particle alias from B524 pad table
// #define PIN_MOTPWM2     // TODO: P0.24 — confirm Particle alias from B524 pad table
// #define PIN_VMOTEN      // TODO: P0.13 — confirm Particle alias from B524 pad table

// Motor current sense (MCP6006T op-amp output → B524 ADC input)
// Schematic: IM = ADC2
// VERIFY: "ADC2" on schematic — confirm which Particle A-pin this corresponds to.
// #define PIN_MOTOR_CURRENT_SENSE   // TODO: ADC2 — likely A2 = P0.28, but A2 = SWOUT; verify


// -----------------------------------------------------------------------------
// Temperature Sensor — DS18B20
// Schematic: 1WIRE = P0.07
// VERIFY: P0.07 does not map to a standard D/A alias on B524.
// NOTE: DS18B20 driver on feature/temp_sensor_driver incorrectly uses D2 (P1.01).
//       Fix that driver to use PIN_1WIRE once alias is confirmed.
// pins_init() skips this until alias is confirmed.
// -----------------------------------------------------------------------------
// #define PIN_1WIRE       // TODO: P0.07 — confirm Particle alias from B524 pad table


// -----------------------------------------------------------------------------
// LCD Display — ER-TFT2.79-1 (NV3007 controller)
// Schematic: CS=P1.08, RS(D/C)=P1.09, RSTB=P0.11
// -----------------------------------------------------------------------------
#define PIN_LCD_CS      D4      // P1.08 — chip select, active-low
#define PIN_LCD_DC      D10     // P1.09 — data/command: LOW=cmd, HIGH=data
                                // NOTE: D10 on B524 = P1.09; verify this alias exists
// #define PIN_LCD_RST  // TODO: P0.11 — confirm Particle alias (not standard D/A range)

// LCD Backlight — BSS138PW MOSFET gate, PWM
// Schematic: LCD_BL = P1.01
#define PIN_LCD_BL      D2      // P1.01 — backlight PWM, active-high gate drive


// -----------------------------------------------------------------------------
// Buzzer — CMI-1295-03TH (2.7 kHz resonant frequency)
// Schematic: BZR = P1.04
// VERIFY: P1.04 does not map to a standard D/A alias on B524.
// NOTE: Buzzer driver on feature/Buzzer_PWM_Driver uses D6 (P1.11) — incorrect.
// pins_init() skips this until alias is confirmed.
// -----------------------------------------------------------------------------
// #define PIN_BUZZER    // TODO: P1.04 — confirm Particle alias from B524 pad table


// -----------------------------------------------------------------------------
// Buttons — Membrane keyboard
// Schematic: BTN_1=P0.31, BTN_2=P0.05
// All button inputs are active-low; configure with INPUT_PULLUP.
// OPEN: BTN_PWR and BTN_3 pin numbers not in project plan — add when known.
// -----------------------------------------------------------------------------
#define PIN_BTN_1       A5      // P0.31 — active-low
#define PIN_BTN_2       A1      // P0.05 — active-low
// #define PIN_BTN_PWR  // TODO: pin not listed in project plan — confirm with schematic
// #define PIN_BTN_3    // TODO: pin not listed in project plan — confirm with schematic


// -----------------------------------------------------------------------------
// RGB LED — Membrane keyboard (PWM)
// Schematic: R=P0.16, G=P0.15, B=P0.14
// VERIFY: P0.14–P0.16 do not map to standard D/A aliases on B524.
// pins_init() skips these until aliases are confirmed.
// -----------------------------------------------------------------------------
// #define PIN_RGB_R    // TODO: P0.16 — confirm Particle alias from B524 pad table
// #define PIN_RGB_G    // TODO: P0.15 — confirm Particle alias from B524 pad table
// #define PIN_RGB_B    // TODO: P0.14 — confirm Particle alias from B524 pad table


// -----------------------------------------------------------------------------
// Power Monitor — PAC1951T-1E/4MX
// Schematic: ALERT1=P1.03, ALERT2=ADC1, PWRDN=P0.30
// I2C address: 0x10 (ADDR pin tied to GND via 0Ω resistor)
// NOTE: Project plan table incorrectly lists addr as 0x4B — that is MP2672AGD's addr.
// -----------------------------------------------------------------------------
#define PIN_PAC_ALERT1      D8      // P1.03 — open-drain interrupt, active-low
#define PIN_PAC_ALERT2      A3      // ADC1 → A3 = P0.29; VERIFY "ADC1" schematic label
#define PIN_PAC_PWRDN       A4      // P0.30 — active-low power-down (HIGH = normal op)
#define I2C_ADDR_PAC1951T   0x10


// -----------------------------------------------------------------------------
// Battery Charger — MP2672AGD
// Interface: I2C
// I2C address: 0x4B (confirmed in driver)
// -----------------------------------------------------------------------------
#define I2C_ADDR_MP2672AGD  0x4B


// -----------------------------------------------------------------------------
// Power Switch — XC6192AA10ER-G
// Schematic: PWR_SHDN=P0.02, SWOUT=P0.28
// VERIFY: P0.02 — check if exposed on B524 (may conflict with reset/NFC logic).
// pins_init() skips PWR_SHDN until confirmed safe.
// -----------------------------------------------------------------------------
// #define PIN_PWR_SHDN // TODO: P0.02 — confirm safe to use as GPIO on B524
#define PIN_SWOUT       A2      // P0.28 — wake signal from power button controller


// -----------------------------------------------------------------------------
// I2C Bus 0 — shared by MP2672AGD and PAC1951T
// -----------------------------------------------------------------------------
#define PIN_I2C_SDA     D0      // P0.26
#define PIN_I2C_SCL     D1      // P0.27


// -----------------------------------------------------------------------------
// SPI Bus 0 — used by LCD (NV3007 controller)
// For hal_selftest() loopback: short MOSI (P1.13) to MISO (P1.14) via jumper,
// assert PIN_LCD_CS during the transfer.
// -----------------------------------------------------------------------------
#define PIN_SPI_SCK     SCK     // P1.15
#define PIN_SPI_MOSI    MOSI    // P1.13
#define PIN_SPI_MISO    MISO    // P1.14


// -----------------------------------------------------------------------------
// Debug UART — J502 header (hardware UART, Serial1)
// -----------------------------------------------------------------------------
#define PIN_DEBUG_TX    TX      // P0.06
#define PIN_DEBUG_RX    RX      // P0.08
