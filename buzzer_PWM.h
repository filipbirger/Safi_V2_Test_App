#ifndef BUZZER_PWM_H
#define BUZZER_PWM_H

#include "Particle.h"

//CMI-1295-03TH buzzer on nRF52840 P1.04
//confirm which Particle pin label corresponds to P1.04 on the B524 schematic (it is D6)
#define BUZZER_PIN D6

// Audible range for testing (Hz)
#define BUZZER_FREQ_MIN     2100
#define BUZZER_FREQ_MAX     2700
#define BUZZER_FREQ_DEFAULT 2400
#define BUZZER_BEEP_MS      200 //default beep duration -> can play with this during testing

void buzzer_init();                          // configure pin, leave buzzer off
void buzzer_tone(uint16_t hz, uint16_t ms); // play tone at hz for ms, non-blocking
void buzzer_beep();                          // single default beep, non-blocking

#endif // BUZZER_PWM_H
