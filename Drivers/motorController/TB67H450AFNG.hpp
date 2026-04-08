#pragma once
#include "Particle.h"

// =========================
// User config
// =========================
#define MOTPWM1        D2      // IN1
#define MOTPWM2        D3      // IN2
#define PIN_IM         A1      // Current sense ADC input (U2 OUT → IM → B524 pin 35)

// ADC reference
#define ADC_MAX_COUNTS  4095
#define ADC_REF_MV      3300

// these values are from the schematic (page 3, Motor driver section)
// R27 = 100mΩ  shunt resistor (RS pin of U6 to GND)
// R10 = 10k    input resistor  (IN+ of U2)
// R9  = 100k   feedback resistor (IN- of U2 to GND)
// Gain = 1 + R9/R10 = 1 + 100k/10k = 11
// No bias — output is 0V at 0A
#define CSENSE_SHUNT_MOHM   100
#define CSENSE_GAIN         11
#define CSENSE_VREF_MV      3300
#define CSENSE_TRIP_MA      2050

// Standby timing from TB67H450 datasheet section 8.2
#define MOTOR_STANDBY_ENTER_MS    2

typedef enum {
    MOTOR_DIR_FORWARD = 0,
    MOTOR_DIR_REVERSE
} MotorDirection_t;

typedef enum {
    MOTOR_COAST = 0,
    MOTOR_BRAKE
} MotorStopMode_t;

typedef struct {
    int16_t  temperature_C;
    uint16_t rampTimeMs;
} RampPoint_t;

// Public API
void     Motor_Init(uint32_t pwmFreqHz);
void     Motor_SetSpeed(uint8_t speed, MotorDirection_t dir);
void     Motor_Stop(MotorStopMode_t mode);
void     Motor_RampProfile(uint8_t targetSpeed, MotorDirection_t dir, const RampPoint_t *profile, uint8_t numPoints);
bool     Motor_IsOvercurrent(void);
uint8_t  Motor_GetSpeed(void);

uint16_t CurrentSense_ReadRaw(void);
int16_t  CurrentSense_GetCurrentmA(void);
int16_t  CurrentSense_GetAveragedmA(uint8_t numSamples);

void     Power_SetMotorSupply(bool enable);
int16_t  Board_GetTemperatureC(void);