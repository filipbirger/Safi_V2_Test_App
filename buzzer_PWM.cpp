//Driver for CMI-1295-03TH PWM buzzer for the Particle B524
//noTone, tone, & pinMode are part of the Particle Device OS API
//pinMode: sets a GPIO pin as input
//tone: starts PWM square wave on a pin
//noTone: stops PWM on a pin

#include "buzzer_PWM.h"

//init guard flag
static bool s_initialized = false;

static void buzzerOff();
static Timer buzzerTimer(100, buzzerOff, true);

static void buzzerOff() {
    noTone(BUZZER_PIN);
}

bool buzzer_init() {
    noTone(BUZZER_PIN); // ensure buzzer is off before configuring pin
    pinMode(BUZZER_PIN, OUTPUT);
    noTone(BUZZER_PIN);
    s_initialized = true;
    return true;
}

bool buzzer_tone(uint16_t hz, uint16_t ms) {
    if (!s_initialized) return false;
    if (hz < BUZZER_FREQ_MIN || hz > BUZZER_FREQ_MAX) return false;
    tone(BUZZER_PIN, hz);
    buzzerTimer.changePeriod(ms);
    buzzerTimer.reset();
    return true;
}

bool buzzer_beep() {
    return buzzer_tone(BUZZER_FREQ_DEFAULT, BUZZER_BEEP_MS);
}
