#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H

#include "Particle.h"

//button pins (external pull-ups fitted, active LOW)
//P0.30 = A4, P0.05 = A6, P0.02 = A7
#define BTN1_PIN    A4
#define BTN2_PIN    A6
#define BTN3_PIN    A7

//timing -> determine length during testing
#define BTN_DEBOUNCE_MS     20      //ms stable before a transition is accepted
#define BTN_LONG_PRESS_MS   500     //ms held to count as a long press

typedef enum {
    BTN_NONE   = 0,
    BTN1_SHORT = 1,
    BTN1_LONG  = 2,
    BTN2_SHORT = 3,
    BTN2_LONG  = 4,
    BTN3_SHORT = 5,
    BTN3_LONG  = 6
} btn_event_t;

void        btn_init();  //configure pins
btn_event_t btn_poll();  //call from main loop; returns event or BTN_NONE

#endif // BUTTON_INPUT_H
