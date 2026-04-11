//Driver for CMI-1295-03TH PWM buzzer for the Particle B524

#include "buzzer_PWM.h"

static void buzzerOff();
static Timer buzzerTimer(100, buzzerOff, true);

static void buzzerOff() {
    noTone(BUZZER_PIN);
}

void buzzer_init() {
    pinMode(BUZZER_PIN, OUTPUT);
    noTone(BUZZER_PIN);
}

void buzzer_tone(uint16_t hz, uint16_t ms) {
    if (hz < BUZZER_FREQ_MIN || hz > BUZZER_FREQ_MAX) return;
    tone(BUZZER_PIN, hz);
    buzzerTimer.changePeriod(ms);
    buzzerTimer.reset();
}

void buzzer_beep() {
    buzzer_tone(BUZZER_FREQ_DEFAULT, BUZZER_BEEP_MS);
}
