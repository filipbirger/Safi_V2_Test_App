//Battery Chrger Driver for MP2672AGD Battery Charger
#include "MP2672AGD.h"

//add to init to set our wanted default values once we know them
//add to read to refresh every single time

/*
 * Charger_Read_Reg
 * Writes the register address then reads back one byte.
 * Returns true on success.
 */
bool Charger_Read_Reg(uint8_t reg, uint8_t *data) {
    Wire.beginTransmission(MP2672_I2C_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;/* NACK or bus error */
    if (Wire.requestFrom(MP2672_I2C_ADDR, (uint8_t)1) != 1) return false;
    *data = (uint8_t)Wire.read();
    Charger_Write_Reg(reg, *data); /* write back read value to refresh the register */
    return true;
}

/*
* Charger_Write_Reg
* Writes one byte to the specified register.
* Returns true on success.
*/
bool Charger_Write_Reg(uint8_t reg, uint8_t data) {
    Wire.beginTransmission(MP2672_I2C_ADDR);
    Wire.write(reg);
    Wire.write(data);
    return (Wire.endTransmission() == 0);
}

/*
* Charger_Init
* Starts the I2C bus and confirms the MP2672AGD is present by reading REG03.
* Returns true if the device ACKs.
*/

bool Charger_Init(void) {
    Wire.begin();
    uint8_t dummy;
    return Charger_Read_Reg(MP2672_REG03, &dummy);
}

/*
 * Charger_GetStatus
 * Reads REG03 (status register) and unpacks all status bits into the
 * provided ChargerStatus_t struct.
 * Returns true on successful I2C read.
 */

bool Charger_GetStatus(ChargerStatus_t *status) {
    if (status == NULL) return false;
    uint8_t raw;
    if (!Charger_Read_Reg(MP2672_REG03, &raw)) return false;

    status->chargeState = (ChargeState_t)((raw & REG03_CHG_STAT_MASK) >> REG03_CHG_STAT_SHIFT);
    status->inPPM       = (raw & REG03_PPM_STAT)       != 0;
    status->battMissing = (raw & REG03_BATTFLOAT_STAT)  != 0;
    status->thermalReg  = (raw & REG03_THERM_STAT)      != 0;
    status->inVsysMin   = (raw & REG03_VSYS_STAT)       != 0;
    return true;
}

/*
* Charger_SetEnabled
* Enables or disables charging via REG00 bit[4] (CHG_CONFIG).
*/

bool Charger_SetEnabled(bool enable) {
    uint8_t reg;
    if (!Charger_Read_Reg(MP2672_REG00, &reg)){
        return false;
    }
    if (enable) {
        reg |=  REG00_CHG_CONFIG;
    }
    else {
        reg &= ~REG00_CHG_CONFIG;
    }
    return Charger_Write_Reg(MP2672_REG00, reg);
}

/*
* Charger_SetChargeCurrent
* Sets the fast charge current via REG01 bits[3:0] (ICC[3:0]).
* Assumes R_ISET = 6kΩ:  offset = 500mA, LSB = 100mA, range = 500–2000mA.
*/
bool Charger_SetChargeCurrent(uint16_t currentMa) {
    if (currentMa < MP2672_ICC_MIN_MA) currentMa = MP2672_ICC_MIN_MA;
    if (currentMa > MP2672_ICC_MAX_MA) currentMa = MP2672_ICC_MAX_MA;
    uint8_t code = (currentMa - MP2672_ICC_OFFSET_MA) / MP2672_ICC_LSB_MA;
    uint8_t reg;
    if (!Charger_Read_Reg(MP2672_REG01, &reg)) return false;  /* preserve NTC_TYPE, VCELL_BAL, balance thresholds */
    reg = (reg & ~REG01_ICC_MASK) | (code & REG01_ICC_MASK);
    return Charger_Write_Reg(MP2672_REG01, reg);
}

/*
* Charger_KickWatchdog
* Resets the I2C watchdog timer via REG02 bit[6] (I2C_WD_TIMER_RESET).
*/

bool Charger_KickWatchdog(void) {
    uint8_t reg;
    if (!Charger_Read_Reg(MP2672_REG02, &reg)) return false;
    reg |= REG02_WD_TIMER_RESET;
    return Charger_Write_Reg(MP2672_REG02, reg);
}

/*
* Charger_Reset
* Resets all registers to default values via REG02 bit[3] (REGISTER_RESET).
*/

bool Charger_Reset(void) {
    uint8_t reg;
    if (!Charger_Read_Reg(MP2672_REG02, &reg)) return false;
    reg |= REG02_REG_RESET;
    return Charger_Write_Reg(MP2672_REG02, reg);
}

/*
* Charger_SetBattRegVoltage
* Sets the battery regulation voltage via REG00 bits[7:5] (VBATT_REG[2:0]).
* Each code step corresponds to a specific voltage (e.g. 0=8.0V, 7=9.5V).
* added this one in
*/
bool Charger_SetBattRegVoltage(uint8_t code) {
    if (code > 7) return false; //Invalid code
    uint8_t reg;
    if (!Charger_Read_Reg(MP2672_REG00, &reg)) return false;
    reg = (reg & ~REG00_VBATT_REG_MASK) | ((code << REG00_VBATT_REG_SHIFT) & REG00_VBATT_REG_MASK);
    return Charger_Write_Reg(MP2672_REG00, reg);
}


/*
* Charger_SetInputCurrentLimit
* Not possible?
*/

// bool Charger_SetInputCurrentLimit(uint16_t currentMa) {

// }/
