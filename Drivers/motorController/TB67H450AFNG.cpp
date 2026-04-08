#include "motor_driver.h"

// =========================
// Private state
// =========================
static uint8_t g_motorSpeed = 0;
static MotorDirection_t g_motorDir = MOTOR_DIR_FORWARD;
static uint32_t g_pwmFreqHz = 25000; // default 25 kHz

// Clamp helper
static uint8_t clampU8(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return (uint8_t)value;
}

// Convert 0-255 duty to Particle analogWrite value
static inline uint8_t pwmDutyToWrite(uint8_t speed) {
    return speed;
}

// Write motor pins for direct PWM mode
static void Motor_WriteOutputs(uint8_t pwmVal, MotorDirection_t dir) {
    if (dir == MOTOR_DIR_FORWARD) {
        analogWrite(MOTPWM1, pwmVal, g_pwmFreqHz);
        analogWrite(MOTPWM2, 0, g_pwmFreqHz);
    } else {
        analogWrite(MOTPWM1, 0, g_pwmFreqHz);
        analogWrite(MOTPWM2, pwmVal, g_pwmFreqHz);
    }
}

// Piecewise-linear lookup for ramp time based on temperature
static uint16_t Motor_GetRampTimeFromProfile(int16_t temperatureC, const RampPoint_t *profile, uint8_t numPoints) {
    if ((profile == nullptr) || (numPoints == 0)) {
        return 0;
    }

    if (numPoints == 1) {
        return profile[0].rampTimeMs;
    }

    if (temperatureC <= profile[0].temperature_C) {
        return profile[0].rampTimeMs;
    }

    for (uint8_t i = 0; i < (numPoints - 1); i++) {
        int16_t  t0 = profile[i].temperature_C;
        int16_t  t1 = profile[i + 1].temperature_C;
        uint16_t r0 = profile[i].rampTimeMs;
        uint16_t r1 = profile[i + 1].rampTimeMs;

        if (temperatureC <= t1) {
            if (t1 == t0) {
                return r1;
            }
            float frac = (float)(temperatureC - t0) / (float)(t1 - t0);
            float ramp = r0 + frac * ((float)r1 - (float)r0);
            return (uint16_t)(ramp + 0.5f);
        }
    }

    return profile[numPoints - 1].rampTimeMs;
}

void Motor_Init(uint32_t pwmFreqHz) {
    g_pwmFreqHz = pwmFreqHz;

    pinMode(MOTPWM1, OUTPUT);
    pinMode(MOTPWM2, OUTPUT);
    pinMode(PIN_IM, INPUT);

    Power_SetMotorSupply(true);

    digitalWrite(MOTPWM1, LOW);
    digitalWrite(MOTPWM2, LOW);

    g_motorSpeed = 0;
    g_motorDir   = MOTOR_DIR_FORWARD;
}

void Motor_SetSpeed(uint8_t speed, MotorDirection_t dir) {
    g_motorSpeed = speed;
    g_motorDir   = dir;

    if (speed == 0) {
    analogWrite(MOTPWM1, 0, g_pwmFreqHz);
    analogWrite(MOTPWM2, 0, g_pwmFreqHz);
    return;
}

    uint8_t pwmVal = pwmDutyToWrite(speed);
    Motor_WriteOutputs(pwmVal, dir);
}

void Motor_Stop(MotorStopMode_t mode) {
    g_motorSpeed = 0;

    if (mode == MOTOR_BRAKE) {
        pinMode(MOTPWM1, OUTPUT);
        pinMode(MOTPWM2, OUTPUT);
        digitalWrite(MOTPWM1, HIGH);
        digitalWrite(MOTPWM2, HIGH);
    } else {
        analogWrite(MOTPWM1, 0, g_pwmFreqHz);
        analogWrite(MOTPWM2, 0, g_pwmFreqHz);
    }
}

void Motor_RampProfile(uint8_t targetSpeed, MotorDirection_t dir, const RampPoint_t *profile, uint8_t numPoints) {
    int16_t  tempC     = Board_GetTemperatureC();
    uint16_t rampTimeMs = Motor_GetRampTimeFromProfile(tempC, profile, numPoints);

    uint8_t startSpeed = g_motorSpeed;

    if ((g_motorSpeed > 0) && (dir != g_motorDir)) {
        Motor_Stop(MOTOR_BRAKE);
        delay(50);
        startSpeed = 0;
    }

    if (rampTimeMs == 0 || startSpeed == targetSpeed) {
        Motor_SetSpeed(targetSpeed, dir);
        return;
    }

    // If starting from rest, IC may be in standby — wait for outputs
    if (startSpeed == 0) {
        delayMicroseconds(30);
    }

    const uint16_t stepPeriodMs = 10;
    uint16_t steps = rampTimeMs / stepPeriodMs;
    if (steps == 0) {
        steps = 1;
    }

    for (uint16_t i = 1; i <= steps; i++) {
        float alpha = (float)i / (float)steps;
        int speed = (int)((1.0f - alpha) * startSpeed + alpha * targetSpeed + 0.5f);
        Motor_SetSpeed((uint8_t)speed, dir);

        // Overcurrent check every step — abort and brake if tripped
        if (Motor_IsOvercurrent()) {
            Motor_Stop(MOTOR_BRAKE);
            return;
        }

        delay(stepPeriodMs);
    }

    Motor_SetSpeed(targetSpeed, dir);
}

bool Motor_IsOvercurrent(void) {
    int16_t mA    = CurrentSense_GetAveragedmA(8);
    int16_t absMa = (mA < 0) ? -mA : mA;
    return (absMa > CSENSE_TRIP_MA);
}

uint8_t Motor_GetSpeed(void) {
    return g_motorSpeed;
}

uint16_t CurrentSense_ReadRaw(void) {
    // Single 12-bit ADC read from PIN_IM (MCP6006 output)
    // 0A  → ADC ~0
    // 2.05A → ADC ~2799
    return (uint16_t)analogRead(PIN_IM);
}

int16_t CurrentSense_GetCurrentmA(void) {
    uint16_t raw = CurrentSense_ReadRaw();

    // Step 1: counts → millivolts
    // V = (raw × 3300mV) / 4096    
    int32_t vOutMv = ((int32_t)raw * CSENSE_VREF_MV) / ADC_MAX_COUNTS;

    // Step 2: undo op-amp gain to get shunt voltage
    // V_shunt = V_out / gain (gain = 11, from 1 + R9/R10)
    int32_t vShuntMv = vOutMv / CSENSE_GAIN;

    // Step 3: shunt voltage → current
    // I(mA) = V_shunt(mV) × 1000 / R_shunt(mΩ)
    //       = V_shunt(mV) × 1000 / 100
    //       = V_shunt(mV) × 10
    int32_t iMa = (vShuntMv * 1000) / CSENSE_SHUNT_MOHM;

    if (iMa >  32767) iMa =  32767;
    if (iMa < -32768) iMa = -32768;

    return (int16_t)iMa;
}

int16_t CurrentSense_GetAveragedmA(uint8_t numSamples) {
    if (numSamples == 0) numSamples = 1;
    if (numSamples > 64) numSamples = 64;

    // Accumulate raw ADC readings
    int32_t sum = 0;
    for (uint8_t i = 0; i < numSamples; i++) {
        sum += (int32_t)analogRead(PIN_IM);
        delayMicroseconds(50);  // ~20 kSa/s inter-sample gap
    }

    uint16_t avg = (uint16_t)(sum / numSamples);

    // Same conversion as GetCurrentmA but on averaged raw count
    int32_t vOutMv   = ((int32_t)avg * CSENSE_VREF_MV) / ADC_MAX_COUNTS;
    int32_t vShuntMv = vOutMv / CSENSE_GAIN;
    int32_t iMa      = (vShuntMv * 1000) / CSENSE_SHUNT_MOHM;

    if (iMa >  32767) iMa =  32767;
    if (iMa < -32768) iMa = -32768;

    return (int16_t)iMa;
}