//Divyesh Bhargava
//Battery Monitor Driver for PAC1951T-1 (Single-Channel High-Side Power Monitor)
#include "PAC1951T.h"

//add to init to set our wanted default values
//add to read to refresh every single time

/*
* BattMon_SendCommand
* Sends a bare command byte with no register address (SMBus Send Byte).
* Used for REFRESH, REFRESH_V, REFRESH_G.
*/

static bool BattMon_SendCommand(uint8_t cmd) {
    Wire.beginTransmission(PAC1951_I2C_ADDR);
    Wire.write(cmd);
    return (Wire.endTransmission() == 0);
}

/* Raw I2C access */

/*
* BattMon_ReadReg
* Points to the register then reads back len bytes (MSB first).
*/
bool BattMon_ReadReg(uint8_t reg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission(PAC1951_I2C_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(PAC1951_I2C_ADDR, (uint8_t)len) != len) return false;
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = (uint8_t)Wire.read();
    }
    return true;
}

/*
* BattMon_WriteReg
* Writes len bytes to the given register address.
*/
bool BattMon_WriteReg(uint8_t reg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission(PAC1951_I2C_ADDR);
    Wire.write(reg);
    for (uint8_t i = 0; i < len; i++) {
        Wire.write(buf[i]);
    }
    return (Wire.endTransmission() == 0);
}

/* Functions */

/*
* BattMon_Init
* Checks PID/MID to confirm the right chip is there, then sets up
* bipolar current sensing and sends an initial REFRESH.
*/

bool BattMon_Init(void) {
    Wire.begin();

    uint8_t pid, mid;
    //confirm the chip is connected reading the pid and mid registers
    if (!BattMon_ReadReg(PAC1951_REG_PID, &pid, 1)) return false;
    if (!BattMon_ReadReg(PAC1951_REG_MID, &mid, 1)) return false;
    if (pid != PAC1951_EXPECTED_PID || mid != PAC1951_EXPECTED_MID) return false;

    //send refresh to latch initial results (trigger measurement snapshot)
    if (!BattMon_SendCommand(PAC1951_CMD_REFRESH)) return false;

    //configure bidirectional (bipolar) mode for VSENSE1:
    //NEG_PWR_FSR (0x1D): 0 = unidirectional (0 to +100mV), 1 = bidirectional (±100mV)
    //Bit 15 (CH1_BIDI) = 1 → VSENSE1 is bipolar (-100mV to +100mV).
    //default = 0 (all unipolar). Write 0x8000 to enable Ch1 bipolar.
    uint8_t neg_pwr[2] = {0x80, 0x00}; //MSB first: 0x8000
    if (!BattMon_WriteReg(PAC1951_REG_NEG_PWR_FSR, neg_pwr, 2)) return false;
    //config registers only take effect after a refresh, so send a command to make the bipolar setting active
    return BattMon_SendCommand(PAC1951_CMD_REFRESH);
}

/*
* BattMon_GetVoltagemV
* Reads VBUS1 (0x07). mV = raw * 32000 / 65536.
*/

uint16_t BattMon_GetVoltagemV(void) {
    uint8_t buf[2];
    if (!BattMon_ReadReg(PAC1951_REG_VBUS1, buf, 2)) return 0;
    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1]; //MSB first
    return (uint16_t)((uint32_t)raw * PAC1951_VBUS_FSR_MV / PAC1951_ADC_FULL_SCALE);
}

/*
* BattMon_GetCurrentmA
* Reads VSENSE1 (0x0B), signed. currentmA = raw * 100000 / (32768 * 33).
* Positive = charging, negative = discharging.
*/

int16_t BattMon_GetCurrentmA(void) {
    uint8_t buf[2];
    if (!BattMon_ReadReg(PAC1951_REG_VSENSE1, buf, 2)) return 0;
    int16_t raw = (int16_t)(((uint16_t)buf[0] << 8) | buf[1]); //MSB first, signed
    return (int16_t)((int32_t)raw * 100000L / (32768L * PAC1951_RSENSE_MOHM));
}

/*
* BattMon_GetPowermW
* Reads VPOWER1 (0x17), 4 bytes. Result is 28-bit (upper bits), so shift right 4 first.
* FSR = 32V * 100mV / 33mΩ = 96969.7mW.
*/

uint32_t BattMon_GetPowermW(void) {
    uint8_t buf[4];
    if (!BattMon_ReadReg(PAC1951_REG_VPOWER1, buf, 4)) return 0;
    uint32_t raw = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | buf[3]; //MSB first
    raw = raw >> 4; //VPOWER1 is a 28-bit value in the upper 28 bits of the 32-bit register
    return (uint32_t)((uint64_t)raw * PAC1951_VBUS_FSR_MV / PAC1951_ADC_FULL_SCALE);
}

/*
* BattMon_GetEnergymWh
* Reads VACC1 (0x03, 7 bytes) and ACC_COUNT (0x02, 4 bytes).
* Energy = (vacc * P_FSR) / (count * 3686400) — assumes 1024 SPS default sample rate.
*/

float BattMon_GetEnergymWh(void) {
    uint8_t vacc_buf[7];
    uint8_t count_buf[4];
    if (!BattMon_ReadReg(PAC1951_REG_VACC1, vacc_buf, 7)) return 0.0f;
    if (!BattMon_ReadReg(PAC1951_REG_ACC_COUNT, count_buf, 4)) return 0.0f;

    /* VACC is a 56-bit value in the upper 56 bits of the 64-bit register */
    uint64_t vacc = ((uint64_t)vacc_buf[0] << 48) | ((uint64_t)vacc_buf[1] << 40) |
                    ((uint64_t)vacc_buf[2] << 32) | ((uint64_t)vacc_buf[3] << 24) |
                    ((uint64_t)vacc_buf[4] << 16) | ((uint64_t)vacc_buf[5] << 8) |
                    (uint64_t)vacc_buf[6];

     /* ACC_COUNT is a 32-bit value */
     uint32_t count = ((uint32_t)count_buf[0] << 24) | ((uint32_t)count_buf[1] << 16) |
                      ((uint32_t)count_buf[2] << 8) | (uint32_t)count_buf[3];

    if (count == 0) return 0.0f; //avoid division by zero

    /* P_FSR in mW = (VBUS_FSR_mV * VSENSE_FSR_uV) / (1000 * Rsense_mΩ) */
    float p_fsr_mw = (float)PAC1951_VBUS_FSR_MV * (float)PAC1951_VSENSE_FSR_UV /
                     (1000.0f * (float)PAC1951_RSENSE_MOHM);
    return ((float)vacc * p_fsr_mw) / ((float)count * (float)PAC1951_ACC_TICKS_PER_HOUR);
}

/*
* BattMon_ResetAccumulator
* Sends REFRESH — latches results and zeroes VACC + ACC_COUNT.
* Call this at full charge to start a fresh energy measurement window.
*/
bool BattMon_ResetAccumulator(void) {
    return BattMon_SendCommand(PAC1951_CMD_REFRESH);
}

/*
* BattMon_GetAvgVoltagemV
* Reads VBUS1_AVG (0x0F). Hardware average of last 8 samples, same scaling as GetVoltagemV.
*/
uint16_t BattMon_GetAvgVoltagemV(void) {
    uint8_t buf[2];
    if (!BattMon_ReadReg(PAC1951_REG_VBUS1_AVG, buf, 2)) return 0;
    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    return (uint16_t)((uint32_t)raw * PAC1951_VBUS_FSR_MV / PAC1951_ADC_FULL_SCALE);
}

/*
* BattMon_GetAvgCurrentmA
* Reads VSENSE1_AVG (0x13). Hardware average of last 8 samples, same scaling as GetCurrentmA.
*/
int16_t BattMon_GetAvgCurrentmA(void) {
    uint8_t buf[2];
    if (!BattMon_ReadReg(PAC1951_REG_VSENSE1_AVG, buf, 2)) return 0;
    int16_t raw = (int16_t)(((uint16_t)buf[0] << 8) | buf[1]);
    return (int16_t)((int32_t)raw * 100000L / (32768L * PAC1951_RSENSE_MOHM));
}

/*
* BattMon_SetOVLimit
* Converts limitMv to a register value and writes to OV1_LIMIT (0x3C).
* reg_val = limitMv * 65536 / 32000
*/
bool BattMon_SetOVLimit(uint16_t limitMv) {
    uint16_t reg_val = (uint16_t)((uint32_t)limitMv * PAC1951_ADC_FULL_SCALE / PAC1951_VBUS_FSR_MV);
    uint8_t buf[2] = {(uint8_t)(reg_val >> 8), (uint8_t)(reg_val & 0xFF)};
    return BattMon_WriteReg(PAC1951_REG_OV1_LIMIT, buf, 2);
}

/*
* BattMon_SetUVLimit
* Same as SetOVLimit but writes to UV1_LIMIT (0x40).
*/
bool BattMon_SetUVLimit(uint16_t limitMv) {
    uint16_t reg_val = (uint16_t)((uint32_t)limitMv * PAC1951_ADC_FULL_SCALE / PAC1951_VBUS_FSR_MV);
    uint8_t buf[2] = {(uint8_t)(reg_val >> 8), (uint8_t)(reg_val & 0xFF)};
    return BattMon_WriteReg(PAC1951_REG_UV1_LIMIT, buf, 2);
}

/*
* BattMon_SetOCLimit
* Converts limitMa to a register value and writes to OC1_LIMIT (0x30).
* reg_val = limitMa * 32768 * 33 / 100000
*/
bool BattMon_SetOCLimit(int16_t limitMa) {
    int16_t reg_val = (int16_t)((int32_t)limitMa * 32768L * PAC1951_RSENSE_MOHM / 100000L);
    uint8_t buf[2] = {(uint8_t)((uint16_t)reg_val >> 8), (uint8_t)(reg_val & 0xFF)};
    return BattMon_WriteReg(PAC1951_REG_OC1_LIMIT, buf, 2);
}

/*
* BattMon_GetAlerts
* Reads ALERT_STATUS (0x26), 3 bytes, and unpacks into the alerts struct.
* Note: this register clears on read — active conditions will re-assert next conversion.
*/
bool BattMon_GetAlerts(BatteryMonAlert_t *alerts) {
    if (alerts == NULL) return false;
    uint8_t buf[3];
    if (!BattMon_ReadReg(PAC1951_REG_ALERT_STATUS, buf, 3)) return false;

    uint32_t raw = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
    alerts->overcurrent  = (raw & ALERT_CH1_OC)    != 0;
    alerts->undercurrent = (raw & ALERT_CH1_UC)    != 0;
    alerts->overvoltage  = (raw & ALERT_CH1_OV)    != 0;
    alerts->undervoltage = (raw & ALERT_CH1_UV)    != 0;
    alerts->overpower    = (raw & ALERT_CH1_OP)    != 0;
    alerts->accumOverflow= (raw & ALERT_ACC_OVF)   != 0;
    alerts->countOverflow= (raw & ALERT_ACC_COUNT)  != 0;
    return true;
}

/*
* BattMon_EnableAlerts
* Writes a bitmask to ALERT_ENABLE (0x49). Use ALERT_CH1_* defines from the header.
* e.g. BattMon_EnableAlerts(ALERT_CH1_OC | ALERT_CH1_OV | ALERT_CH1_UV)
*/
bool BattMon_EnableAlerts(uint32_t alertMask) {
    uint8_t buf[3] = {
        (uint8_t)((alertMask >> 16) & 0xFF),
        (uint8_t)((alertMask >>  8) & 0xFF),
        (uint8_t)( alertMask        & 0xFF)
    };
    return BattMon_WriteReg(PAC1951_REG_ALERT_ENABLE, buf, 3);
}
