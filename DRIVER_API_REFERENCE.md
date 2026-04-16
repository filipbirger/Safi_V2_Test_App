# Safi V2 — Driver API Reference

Quick reference for all driver functions. Update this file when new drivers are added.
Inputs marked with * are pointers (output parameters written by the function).

---

## DS18B20 — Temperature Sensor
**File:** `Drivers/DS18B20.h` | **Interface:** 1-Wire (P0.07) | **Branch:** feature/temp_sensor_driver

| Function | Inputs | Returns | Description |
|---|---|---|---|
| `DS18B20_Init()` | none | `bool` | Starts the 1-Wire bus and checks the sensor is present. Call once in setup(). |
| `DS18B20_Present()` | none | `bool` | Sends a reset pulse and returns true if a presence pulse is detected. |
| `DS18B20_ReadTempC()` | none | `float` | Triggers a temperature conversion (blocking ~750 ms) and returns degrees C. |
| `DS18B20_ReadRaw(raw*)` | `int16_t *raw` | `bool` | Reads the raw 16-bit scratchpad value without converting to float. Returns false if sensor disconnected. |

---

## PAC1951T — Battery Monitor
**File:** `Drivers/PAC1951T.h` | **Interface:** I2C / SMBus (addr 0x10) | **Branch:** feature/battery_charger_driver

| Function | Inputs | Returns | Description |
|---|---|---|---|
| `BattMon_Init()` | none | `bool` | Verifies PID/MID, sends REFRESH command, configures bidirectional current sense. |
| `BattMon_GetVoltagemV()` | none | `uint16_t` | Reads VBUS1 register. Returns battery bus voltage in mV (FSR = 32 V). |
| `BattMon_GetCurrentmA()` | none | `int16_t` | Reads VSENSE1 register. Positive = charging, negative = discharging. In mA. |
| `BattMon_GetPowermW()` | none | `uint32_t` | Reads VPOWER1 register. Returns instantaneous power in mW. |
| `BattMon_GetEnergymWh()` | none | `float` | Reads accumulator + count registers. Returns accumulated energy in mWh since last reset. |
| `BattMon_ResetAccumulator()` | none | `bool` | Sends REFRESH command — latches current readings and clears energy accumulator. |
| `BattMon_GetAvgVoltagemV()` | none | `uint16_t` | Reads hardware-averaged VBUS1. Less noisy than single-shot reading. |
| `BattMon_GetAvgCurrentmA()` | none | `int16_t` | Reads hardware-averaged VSENSE1. Less noisy than single-shot reading. |
| `BattMon_SetOVLimit(limitMv)` | `uint16_t limitMv` | `bool` | Writes overvoltage threshold. Alert fires if VBUS exceeds this value. |
| `BattMon_SetUVLimit(limitMv)` | `uint16_t limitMv` | `bool` | Writes undervoltage threshold. Alert fires if VBUS drops below this value. |
| `BattMon_SetOCLimit(limitMa)` | `int16_t limitMa` | `bool` | Writes overcurrent threshold in mA. |
| `BattMon_GetAlerts(alerts*)` | `BatteryMonAlert_t *alerts` | `bool` | Reads and clears ALERT_STATUS register. Fills struct with OC/UV/OV/OP/accumulator flags. |
| `BattMon_EnableAlerts(mask)` | `uint32_t alertMask` | `bool` | Enables specific alert pins using ALERT_CH1_* bitmask constants. |
| `BattMon_ReadReg(reg, buf*, len)` | `uint8_t reg`, `uint8_t *buf`, `uint8_t len` | `bool` | Raw I2C read. len = 1–7 bytes. For direct register access. |
| `BattMon_WriteReg(reg, buf*, len)` | `uint8_t reg`, `uint8_t *buf`, `uint8_t len` | `bool` | Raw I2C write. len = 1–2 bytes. |

---

## MP2672AGD — Battery Charger
**File:** `Drivers/MP2672AGD.h` | **Interface:** I2C (addr 0x4B) | **Branch:** feature/battery_monitor_driver

| Function | Inputs | Returns | Description |
|---|---|---|---|
| `Charger_Init()` | none | `bool` | Probes device via REG03 read. Returns false if not found on I2C bus. |
| `Charger_GetStatus(status*)` | `ChargerStatus_t *status` | `bool` | Reads REG03. Fills struct with charge state, PPM flag, battery-missing flag, thermal regulation, VSYS status. |
| `Charger_GetFaults(faults*)` | `ChargerFault_t *faults` | `bool` | Reads REG04. Fills struct with watchdog, input OVP, thermal shutdown, timer, battery OVP, and NTC fault flags. |
| `Charger_SetEnabled(enable)` | `bool enable` | `bool` | Sets/clears REG00 bit[4] to enable or disable charging. |
| `Charger_SetChargeCurrentmA(mA)` | `uint16_t currentMa` | `bool` | Sets fast-charge current via REG01 bits[3:0]. Range: 500–2000 mA in 100 mA steps. Clamped automatically. |
| `Charger_SetBattRegVoltage(code)` | `uint8_t code` | `bool` | Sets battery regulation voltage via REG00 bits[7:5]. Code 0–7 maps to 8.3–8.2 V (see header for table). |
| `Charger_KickWatchdog()` | none | `bool` | Resets the I2C watchdog timer by writing REG02 bit[6]. Call periodically if watchdog is enabled. |
| `Charger_Reset()` | none | `bool` | Writes REG02 bit[3] to reset all registers to power-on defaults. |
| `Charger_Read_Reg(reg, data*)` | `uint8_t reg`, `uint8_t *data` | `bool` | Raw single-byte I2C register read. |
| `Charger_Write_Reg(reg, data)` | `uint8_t reg`, `uint8_t data` | `bool` | Raw single-byte I2C register write. |

---

## TB67H450AFNG — Motor Driver
**File:** `Drivers/motorController/TB67H450AFNG.hpp` | **Interface:** PWM + GPIO | **Branch:** feature/motor-controlller-driver

| Function | Inputs | Returns | Description |
|---|---|---|---|
| `Motor_Init(pwmFreqHz)` | `uint32_t pwmFreqHz` | `void` | Configures PWM pins and sets the PWM carrier frequency. Call once in setup(). |
| `Motor_SetSpeed(speed, dir)` | `uint8_t speed` (0–255), `MotorDirection_t dir` | `void` | Sets motor speed and direction. FORWARD or REVERSE from enum. |
| `Motor_Stop(mode)` | `MotorStopMode_t mode` | `void` | Stops motor. MOTOR_COAST (free spin) or MOTOR_BRAKE (active brake). |
| `Motor_RampProfile(speed, dir, profile*, n)` | `uint8_t targetSpeed`, `MotorDirection_t dir`, `const RampPoint_t *profile`, `uint8_t numPoints` | `void` | Ramps motor speed following a temperature-indexed profile array. |
| `Motor_IsOvercurrent()` | none | `bool` | Returns true if ADC current sense reading exceeds CSENSE_TRIP_MA (2050 mA). |
| `Motor_GetSpeed()` | none | `uint8_t` | Returns the last commanded speed (0–255). |
| `CurrentSense_ReadRaw()` | none | `uint16_t` | Returns raw 12-bit ADC count from the current sense pin. |
| `CurrentSense_GetCurrentmA()` | none | `int16_t` | Converts raw ADC reading to milliamps using shunt + amplifier gain. |
| `CurrentSense_GetAveragedmA(n)` | `uint8_t numSamples` | `int16_t` | Takes n ADC samples and returns the averaged current in mA. |
| `Power_SetMotorSupply(enable)` | `bool enable` | `void` | Enables/disables the 6V motor power rail. Must be called before Motor_SetSpeed(). |
| `Board_GetTemperatureC()` | none | `int16_t` | Returns the board/ambient temperature in °C (used by ramp profile). |

---

## Buzzer (CMI-1295-03TH)
**File:** `buzzer_PWM.h` | **Interface:** PWM (P1.04) | **Branch:** feature/Buzzer_PWM_Driver

| Function | Inputs | Returns | Description |
|---|---|---|---|
| `buzzer_init()` | none | `void` | Configures the buzzer pin and leaves it off. Call once in setup(). |
| `buzzer_tone(hz, ms)` | `uint16_t hz`, `uint16_t ms` | `void` | Plays a tone at the given frequency (Hz) for the given duration (ms). Non-blocking. |
| `buzzer_beep()` | none | `void` | Plays a single beep at BUZZER_FREQ_DEFAULT (2400 Hz) for BUZZER_BEEP_MS (200 ms). Non-blocking. |

---

## LCD NV3007 — Display Driver (ER-TFT2.79-1, 142×428)
**File:** `drivers/LCD_NV3007/LCD_NV3007.h` | **Interface:** SPI (CS=P1.08, DC=P1.09, RST=P0.11, BL=P1.01) | **Branch:** feature/lcd_display_driver

### Core

| Function | Inputs | Returns | Description |
|---|---|---|---|
| `LCD_Init()` | none | `bool` | Configures SPI + GPIO, runs NV3007 power-on sequence, clears screen black. Call once in setup(). |
| `LCD_SetBacklight(brightness)` | `uint8_t brightness` (0–255) | `void` | Sets backlight PWM level. 0 = off, 255 = full. |
| `LCD_Clear(colour)` | `uint16_t colour` (RGB565) | `void` | Fills entire 142×428 screen with one colour. |
| `LCD_Sleep()` | none | `void` | Sends display-off + sleep-in commands. Turn backlight off first. |
| `LCD_Wake()` | none | `void` | Sends sleep-out + display-on commands. |

### Primitives

| Function | Inputs | Returns | Description |
|---|---|---|---|
| `LCD_DrawPixel(x, y, colour)` | `uint16_t x, y`, `uint16_t colour` | `void` | Draws a single pixel. Silently clipped at screen edges. |
| `LCD_FillRect(x, y, w, h, colour)` | `uint16_t x, y, w, h`, `uint16_t colour` | `void` | Draws a solid filled rectangle. Clipped to screen bounds. |
| `LCD_DrawHLine(x, y, len, colour)` | `uint16_t x, y, len`, `uint16_t colour` | `void` | Draws a horizontal line. |
| `LCD_DrawVLine(x, y, len, colour)` | `uint16_t x, y, len`, `uint16_t colour` | `void` | Draws a vertical line. |
| `LCD_DrawRect(x, y, w, h, colour)` | `uint16_t x, y, w, h`, `uint16_t colour` | `void` | Draws a hollow rectangle outline. |
| `LCD_DrawChar(x, y, c, fg, bg)` | `uint16_t x, y`, `char c`, `uint16_t fg, bg` | `uint16_t` | Draws one ASCII character (5×7 font). Returns x position after the character. |
| `LCD_DrawString(x, y, str, fg, bg)` | `uint16_t x, y`, `const char *str`, `uint16_t fg, bg` | `void` | Draws a null-terminated string. Wraps to next line at screen edge. |

### Image display

| Function | Inputs | Returns | Description |
|---|---|---|---|
| `LCD_DrawImage(x, y, w, h, pixels)` | `uint16_t x, y, w, h`, `const uint16_t *pixels` | `void` | Blits a raw RGB565 pixel array to an arbitrary screen region. Use LVGL converter output directly. |
| `LCD_DrawStaticUI(pixels)` | `const uint16_t *pixels` | `void` | Draws a 142×300 px image to the top (static) region. Call once after init. |
| `LCD_DrawStateButtons(pixels)` | `const uint16_t *pixels` | `void` | Draws a 142×128 px image to the bottom (state) region. Call on each state transition. |

### Dynamic status elements

| Function | Inputs | Returns | Description |
|---|---|---|---|
| `LCD_DrawBatteryLevel(percent, fill, bg)` | `uint8_t percent` (0–100), `uint16_t fill_colour, bg_colour` | `void` | Redraws battery icon at LCD_BATT_X/Y. Erases old fill then draws proportional bar + outline + terminal nub. |
| `LCD_UpdateTemperature(tempC, fg, bg)` | `float tempC`, `uint16_t fg_colour, bg_colour` | `void` | Erases temperature text field then redraws formatted "XX.X C" string at LCD_TEMP_X/Y. |

### LVGL image converter settings
Convert PNGs at: **True colour (16-bit) · C array output · Swap bytes = YES**

---
