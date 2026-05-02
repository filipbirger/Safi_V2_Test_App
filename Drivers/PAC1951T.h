#ifndef PAC1951T_H
#define PAC1951T_H

#include <stdint.h>
#include <stdbool.h>
#include "Particle.h"

/*
I2C address (0ohm ressitor to GND, so address is 0x10):
*/
#define PAC1951_I2C_ADDR 0x10 

/*
Expected ID values for PAC1951 (used to ensure the device being communicated with is correct)
*/
#define PAC1951_EXPECTED_PID 0x78 //Product ID for PAC1951-1
#define PAC1951_EXPECTED_MID 0x54 //Manufacturer ID for Microchip

/* COMMANDS (sent as a single byte with no register address, using SMBus protocol) */
#define PAC1951_CMD_REFRESH 0x00 //Latch all results + reset accumulators
#define PAC1951_CMD_REFRESH_V 0x1F //Latch results only, do NOT reset accumulators
#define PAC1951_CMD_REFRESH_G 0x1E //Global refresh (all devices on bus)

/* Register addresses */
#define PAC1951_REG_CTRL 0x01 //Control register (R/W, 2 bytes)
#define PAC1951_REG_ACC_COUNT 0x02 //Accumulator count (R, 4 bytes)
#define PAC1951_REG_VACC1 0x03 //Power accumulator Ch1 (R, 7 bytes)
#define PAC1951_REG_VBUS1 0x07 //Bus voltage Ch1 (R, 2 bytes)
#define PAC1951_REG_VSENSE1 0x0B //Sense voltage Ch1 (R, 2 bytes)
#define PAC1951_REG_VBUS1_AVG 0x0F //Bus voltage average
#define PAC1951_REG_VSENSE1_AVG 0x13 //Sense voltage average Ch1 (R, 2 bytes)
#define PAC1951_REG_VPOWER1 0x17 //Power Ch1 (R,4 bytes)
#define PAC1951_REG_SMBUS_SETTINGS 0x1C //SMBus settings (R/W, 1 byte)
#define PAC1951_REG_NEG_PWR_FSR 0x1D //Bipolar/unipolar config (R/W, 2 bytes)
#define PAC1951_REG_CTRL_ACT 0x21 //Active control register shadow (R, 2 bytes)
#define PAC1951_REG_ACCUM_CONFIG 0x25 //Accumulator mode config (R/W, 1 byte)
#define PAC1951_REG_ALERT_STATUS 0x26 //Alert status (RC, 3 bytes) — clears on read
#define PAC1951_REG_SLOW_ALERT1 0x27 //ALERT1 pin routing (R/W, 3 bytes)
#define PAC1951_REG_GPIO_ALERT2 0x28 //ALERT2 pin routing (R/W, 3 bytes)
#define PAC1951_REG_OC1_LIMIT 0x30 //Overcurrent limit Ch1 (R/W, 2 bytes)
#define PAC1951_REG_UC1_LIMIT 0x34 //Undercurrent limit Ch1 (R/W, 2 bytes)
#define PAC1951_REG_OP1_LIMIT 0x38 //Overpower limit Ch1 (R/W, 3 bytes)
#define PAC1951_REG_OV1_LIMIT 0x3C //Overvoltage limit Ch1 (R/W, 2 bytes)
#define PAC1951_REG_UV1_LIMIT 0x40 //Undervoltage limit Ch1 (R/W, 2 bytes)
#define PAC1951_REG_ALERT_ENABLE 0x49 //Alert enable flags (R/W, 3 bytes)
#define PAC1951_REG_PID 0xFD //Product ID (R, 1 byte)
#define PAC1951_REG_MID 0xFE //Manufacturer ID (R, 1 byte)
#define PAC1951_REG_REV 0xFF //Revision ID (R, 1 byte)

/*Alert Status bit masks (REG 0x26, 3 bytes = bits [23:0])*/
/*Byte 0 = bits [23:16], Byte 1 = bits [15:8], Byte 2 = bits [7:0]*/
#define ALERT_CH1_OC (1UL << 23) //Channel 1 overcurrent
#define ALERT_CH1_UC (1UL << 19) //Channel 1 undercurrent
#define ALERT_CH1_OV (1UL << 15) //Channel 1 overvoltage
#define ALERT_CH1_UV (1UL << 11) //Channel 1 undervoltage
#define ALERT_CH1_OP (1UL << 7) //Channel 1 overpower
#define ALERT_ACC_OVF (1UL << 3) //Accumulator overflow
#define ALERT_ACC_COUNT (1UL << 2) //Accumulator count overflow

/*Alert Enable bit masks (REG 0x49, 3 bytes = bits [23:0])*/
#define ALERT_ENABLE_CH1_OC (1UL << 23) //Enable alert for Channel 1 overcurrent
#define ALERT_ENABLE_CH1_UC (1UL << 19) //Enable alert for Channel 1 undercurrent
#define ALERT_ENABLE_CH1_OV (1UL << 15) //Enable alert for Channel 1 overvoltage
#define ALERT_ENABLE_CH1_UV (1UL << 11) //Enable alert for Channel 1 undervoltage
#define ALERT_ENABLE_CH1_OP (1UL << 7) //Enable alert for Channel 1 overpower
#define ALERT_ENABLE_ACC_OVF (1UL << 3) //Enable alert for accumulator overflow
#define ALERT_ENABLE_ACC_COUNT (1UL << 2) //Enable alert for accumulator count overflow

/*Scaling Constants*/
#define PAC1951_VBUS_FSR_MV 32000 //VBUS full-scale range in mV (16-bit unsigned)
#define PAC1951_ADC_FULL_SCALE 65536 //Full scale for 16-bit ADC (2^16)
#define PAC1951_VSENSE_FSR_UV 100000L //VSENSE full-scale range in µV (±100mV = 200mV total range, but bipolar mode uses half-scale for positive range)
#define PAC1951_RSENSE_MOHM 33 //Sense resistor value in mΩ for current calculations (verify with schematic)
#define PAC1951_SAMPLE_RATE_SPS   1024UL
#define PAC1951_ACC_TICKS_PER_HOUR  (PAC1951_SAMPLE_RATE_SPS * 3600UL)

/*Alert struct (ALERT_STATUS, 0x26)*/
typedef struct {
    bool overcurrent; //Channel 1 overcurrent alert
    bool undercurrent; //Channel 1 undercurrent alert
    bool overvoltage; //Channel 1 overvoltage alert
    bool undervoltage; //Channel 1 undervoltage alert
    bool overpower; //Channel 1 overpower alert
    bool accumOverflow; //Accumulator overflow alert
    bool countOverflow; //Accumulator count overflow alert
} BatteryMonAlert_t;

/* ── Function Prototypes ─────────────────────────────────────────────────── */
bool     BattMon_Init(void);                            /* Read PID (0xFD) + MID (0xFE), send REFRESH, config bidirectional */
uint16_t BattMon_GetVoltagemV(void);                    /* Read VBUS1 (0x07), 2 bytes. FSR=32V. Returns mV. */
int16_t  BattMon_GetCurrentmA(void);                    /* Read VSENSE1 (0x0B), 2 bytes. I = Vsense / 33mΩ. +ve=charging */
uint32_t BattMon_GetPowermW(void);                      /* Read VPOWER1 (0x17), 4 bytes. Returns instantaneous mW. */
float    BattMon_GetEnergymWh(void);                    /* Read VACC1 (0x03, 7 bytes) + ACC_COUNT (0x02, 4 bytes). */
bool     BattMon_ResetAccumulator(void);                /* Send REFRESH (0x00). Latches results and resets accumulators. */
uint16_t BattMon_GetAvgVoltagemV(void);                 /* Read VBUS1_AVG (0x0F), 2 bytes. Hardware-averaged VBUS. */
int16_t  BattMon_GetAvgCurrentmA(void);                 /* Read VSENSE1_AVG (0x13), 2 bytes. Hardware-averaged current. */
bool     BattMon_SetOVLimit(uint16_t limitMv);          /* Write OV1_LIMIT (0x3C). Overvoltage threshold in mV. */
bool     BattMon_SetUVLimit(uint16_t limitMv);          /* Write UV1_LIMIT (0x40). Undervoltage threshold in mV. */
bool     BattMon_SetOCLimit(int16_t limitMa);           /* Write OC1_LIMIT (0x30). Overcurrent threshold in mA. */
bool     BattMon_GetAlerts(BatteryMonAlert_t *alerts);  /* Read ALERT_STATUS (0x26), 3 bytes. Clears on read. */
bool     BattMon_EnableAlerts(uint32_t alertMask);      /* Write ALERT_ENABLE (0x49) with ALERT_CH1_* bitmask. */
bool     BattMon_ReadReg(uint8_t reg, uint8_t *buf, uint8_t len);  /* Raw I2C read, 1–7 bytes. */
bool     BattMon_WriteReg(uint8_t reg, uint8_t *buf, uint8_t len); /* Raw I2C write, 1–2 bytes. */

#endif /* PAC1951T_H */