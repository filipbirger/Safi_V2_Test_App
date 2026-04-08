#ifndef MP2672AGD_H
#define MP2672AGD_H

#include <stdint.h>
#include <stdbool.h>
//#include "Particle.h"

//I2C address of the MP2672AGD charger IC
#define MP2672_I2C_ADDR 0x4B

//register addresses
#define MP2672_REG00 0x00 //Battery regulation voltage, charge configuration, and SYS voltage setting register.
#define MP2672_REG01 0x01 //Cell balance and charge current setting register.
#define MP2672_REG02 0x02 //Timer, watchdog, and reset control register
#define MP2672_REG03 0x03 //Status register (read-only)
#define MP2672_REG04 0x04 //Fault register (read-only)

/*---------------------------REG00 Bit masks and codes---------------------------*/
//00111000 default
#define REG00_VBATT_REG_MASK 0xE0 //Bit mask to isolate Bits [7:5], battery regulation voltage
#define REG00_VBATT_REG_SHIFT 5 //get to the LSB of the VBATT_REG field
//VBATT_REG codes: 0=8.3V, 1=8.4V, 2=8.5V, 3=8.6V, 4=8.7V, 5=8.8V, 6=8.9V, 7=8.2V
#define REG00_CHG_CONFIG (1 << 4) //Bit mask to isolate Bit 4, 0=charging disabled, 1=enabled
#define REG00_VBATT_PRE_MASK 0x0E //Bit mask to isolate Bits [3:1], SYS min voltage offset
#define REG00_VBATT_PRE_SHIFT 1 //get to the LSB of the VBATT_PRE field
#define REG00_CELL_OVP_HYS (1 << 0) //Bit mask to isolate Bit 0, 0=80mV, 1=0mV

/*---------------------------REG01 Bit masks and codes---------------------------*/
//10001111 default
#define REG01_NTC_TYPE (1 << 7) //Bit mask to isolate Bit 7, 0=Standard, 1=JEITA
#define REG01_VCELL_BAL (1 << 6) //Bit mask to isolate Bit 6, 0=3.5V, 1=3.7V balance start
#define REG01_BAL_THRESH_H2L (1 << 5) //Bit mask to isolate Bit 5, 0=50mV, 1=70mV
#define REG01_BAL_THRESH_L2H (1 << 4) //Bit mask to isolate Bit 4, 0=50mV, 1=70mV
#define REG01_ICC_MASK 0x0F //Bit mask to isolate Bits [3:0], fast charge current
#define REG01_ICC_SHIFT 0 //get to the LSB of the ICC field

/*---------------------------REG02 Bit masks and codes---------------------------*/
//10010101 default
#define REG02_FSW (1 << 7) //Bit mask to isolate Bit 7, 0=600kHz, 1=1200kHz
#define REG02_WD_TIMER_RESET (1 << 6) //Bit mask to isolate Bit 6, 0 = normal, 1 = reset watchdog timer
#define REG02_WD_TIMER_MASK 0x30 //Bit mask to isolate Bits [5:4], Watchdog timer setting, 00=off, 01=40s, 10=80s, 11=160s
#define REG02_WD_TIMER_SHIFT 4 //get to the LSB of the Watchdog timer field
#define REG02_REG_RESET (1 << 3) //Bit mask to isolate Bit 3, 0 = keep settings, 1 = reset all registers to default (auto-clears)
#define REG02_CHG_TMR_MASK 0x06 //Bit mask to isolate Bits [2:1], Charge timer setting, 00=off, 01=8h, 10=20h, 11=12h
#define REG02_CHG_TMR_SHIFT 1 //get to the LSB of the Charge timer field
#define REG02_EN_SUSP (1 << 0) //Bit mask to isolate Bit 0, 0=suspend (boost off), 1=normal (boost on)

/*---------------------------REG03 (Status) Bit masks and codes---------------------------*/
//00000000 default (Bits 6 and 7 reserved)
#define REG03_CHG_STAT_MASK 0x30 //Bit mask to isolate Bits [5:4], Charge status
#define REG03_CHG_STAT_SHIFT 4 //get to the LSB of the Charge status field
#define REG03_PPM_STAT (1 << 3) //Bit mask to isolate Bit 3, 0 = not in VIN PPM, 1 = in VIN PPM
#define REG03_BATTFLOAT_STAT (1 << 2) //Bit mask to isolate Bit 2, 0 = battery present, 1 = battery missing
#define REG03_THERM_STAT (1 << 1) //Bit mask to isolate Bit 1, 0 = thermal regulation inactive, 1 = thermal regulation active
#define REG03_VSYS_STAT (1 << 0) //Bit mask to isolate Bit 0, 0 = above VSYSMIN, 1 = in VSYSMIN regulation

/*---------------------------REG04 (Fault) Bit masks and codes---------------------------*/
//00000000 default
#define REG04_WD_FAULT (1 << 7) //Bit mask to isolate Bit 7, 0 = watchdog okay, 1 = watchdog expired
#define REG04_INPUT_FAULT (1 << 6) //Bit mask to isolate Bit 6, 0 = input voltage okay, 1 = input OVP
#define REG04_THERMSD_FAULT (1 << 5) //Bit mask to isolate Bit 5, 0 = temperature okay, 1 = thermal shutdown
#define REG04_TIMER_FAULT (1 << 4) //Bit mask to isolate Bit 4, 0 = charge timer okay, 1 = safety timer expired
#define REG04_BAT_FAULT (1 << 3) //Bit mask to isolate Bit 3, 0 = battery voltage okay, 1 = battery OVP
#define REG04_NTC_FAULT_MASK 0x07 //Bit mask to isolate Bits [2:0], NTC fault code
#define REG04_NTC_FAULT_SHIFT 0 //get to the LSB of the NTC fault code field



/*--------------------Charge current scaling (assumes R_ISET = 6kΩ)----------------------*/
//range = 500mA to 2000mA in 100mA steps, encoded as 4-bit value in REG01[3:0], currentMa = ICC * 100mA + 500mA
#define MP2672_ICC_OFFSET_MA 500 //Charge current offset in mA)
#define MP2672_ICC_LSB_MA 100 //Charge current LSB in mA
#define MP2672_ICC_MIN_MA 500 //Minimum charge current in mA
#define MP2672_ICC_MAX_MA 2000 //Maximum charge current in mA

/*Watchdog Timer enum (WD_FAULT[1:0]) from REG02[5:4]*/
typedef enum {
    WD_TIMER_OFF = 0x00, //Watchdog timer off
    WD_TIMER_40S = 0x01, //Watchdog timer 40s
    WD_TIMER_80S = 0x02, //Watchdog timer 80s
    WD_TIMER_160S = 0x03 //Watchdog timer 160s
} WatchdogTimer_t;

/*Charge timer enum (CHG_TMR[1:0]) from REG02[2:1]*/
typedef enum {
    CHG_TIMER_OFF = 0x00, //Charge timer off
    CHG_TIMER_8H = 0x01, //Charge timer 8h
    CHG_TIMER_20H = 0x02, //Charge timer 20h
    CHG_TIMER_12H = 0x03 //Charge timer 12h
} ChargeTimer_t;

/*Charge state enum (CHG_STAT[1:0]) from REG03[5:4]*/
typedef enum {
    CHG_STATE_NOT_CHARGING = 0x00, //No charging
    CHG_STATE_PRE_CHARGE = 0x01, //Pre-charge
    CHG_STATE_FAST_CHARGE = 0x02, //Fast charging
    CHG_STATE_CHARGE_DONE = 0x03 //Charge done
} ChargeState_t;

/*NTC fault enum (NTC_FAULT[2:0]) from REG04[2:0]*/
typedef enum {
    NTC_FAULT_OK = 0x00, //NTC okay
    NTC_FAULT_SHORT_VCC = 0x01, //NTC shorted to VCC
    NTC_FAULT_SHORT_GND = 0x02, //NTC shorted to GND
    NTC_FAULT_ABOVE_THRESHOLD = 0x03, //NTC above threshold (too cold)
    NTC_FAULT_BELOW_THRESHOLD = 0x04, //NTC below threshold (too hot)
    NTC_FAULT_RESERVED_1 = 0x05, //Reserved
    NTC_FAULT_RESERVED_2 = 0x06, //Reserved
    NTC_FAULT_RESERVED_3 = 0x07 //Reserved
} NtcFault_t;

/*Status struct (REG03)*/
typedef struct {
    ChargeState_t chargeState; //Charging state from REG03[5:4]
    bool inPPM; //Input power management mode active (REG03[3])
    bool battMissing; //Battery missing (REG03[2])
    bool thermalReg; //Thermal regulation active (REG03[1])
    bool inVsysMin; //In VSYSMIN regulation (REG03[0])
} ChargerStatus_t;

/*Fault struct (REG04)*/
typedef struct {
    bool watchdogExpired; //Watchdog timer expired (REG04[7])
    bool inputOVP; //Input overvoltage (REG04[6])
    bool thermalShutdown; //Thermal shutdown (REG04[5])
    bool safetyTimerExp; //Safety timer expired (REG04[4])
    bool battOVP; //Battery overvoltage (REG04[3])
    NtcFault_t ntcFault; //NTC fault code from REG04[2:0]
} ChargerFault_t;

/* Function Prototypes */
bool Charger_Init(void);                              /* I2C probe via REG03 read */
bool Charger_GetStatus(ChargerStatus_t *status);      /* Read REG03 → status struct */
bool Charger_GetFaults(ChargerFault_t *faults);       /* Read REG04 → fault struct */
bool Charger_SetEnabled(bool enable);                 /* REG00 bit[4] CHG_CONFIG */
bool Charger_SetChargeCurrentmA(uint16_t currentMa); /* REG01 bits[3:0] ICC, clamped */
bool Charger_SetBattRegVoltage(uint8_t code);         /* REG00 bits[7:5] VBATT_REG (0–7) */
bool Charger_KickWatchdog(void);                      /* REG02 bit[6] I2C_WD_TIMER_RESET */
bool Charger_Reset(void);                             /* REG02 bit[3] REGISTER_RESET */
bool Charger_Read_Reg(uint8_t reg, uint8_t *data);   /* Raw single-byte I2C read */
bool Charger_Write_Reg(uint8_t reg, uint8_t data);   /* Raw single-byte I2C write */

#endif /* MP2672AGD_H */