// Button input driver — 3x GPIO buttons, active low, external pull-ups
// Debounce + short/long press detection via polling

#include "button_input.h"

#define NUM_BUTTONS 3

static const pin_t BTN_PINS[NUM_BUTTONS] = { BTN1_PIN, BTN2_PIN, BTN3_PIN };

static const btn_event_t SHORT_EVENTS[NUM_BUTTONS] = { BTN1_SHORT, BTN2_SHORT, BTN3_SHORT };
static const btn_event_t LONG_EVENTS[NUM_BUTTONS]  = { BTN1_LONG,  BTN2_LONG,  BTN3_LONG  };

typedef struct {
    bool     lastRaw;       // last raw pin reading
    bool     debounced;     // current debounced state
    uint32_t lastChangeMs;  // time of last raw state change (for debounce)
    uint32_t pressStartMs;  // time debounced press began
    bool     pressed;       // true while button is held down
} btn_state_t;

static btn_state_t states[NUM_BUTTONS];

void btn_init() {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        pinMode(BTN_PINS[i], INPUT);   // external pull-ups on schematic
        states[i].lastRaw      = HIGH;
        states[i].debounced    = HIGH;
        states[i].lastChangeMs = 0;
        states[i].pressStartMs = 0;
        states[i].pressed      = false;
    }
}

btn_event_t btn_poll() {
    uint32_t now = millis();

    for (int i = 0; i < NUM_BUTTONS; i++) {
        bool raw = digitalRead(BTN_PINS[i]);

        // Reset debounce window on any raw change
        if (raw != states[i].lastRaw) {
            states[i].lastRaw      = raw;
            states[i].lastChangeMs = now;
        }

        // Not yet stable — skip
        if ((now - states[i].lastChangeMs) < BTN_DEBOUNCE_MS) continue;

        // Stable transition detected
        if (raw != states[i].debounced) {
            states[i].debounced = raw;

            if (raw == LOW) {
                // Falling edge — button pressed
                states[i].pressed      = true;
                states[i].pressStartMs = now;
            } else {
                // Rising edge — button released
                if (states[i].pressed) {
                    states[i].pressed   = false;
                    uint32_t held       = now - states[i].pressStartMs;
                    return (held >= BTN_LONG_PRESS_MS) ? LONG_EVENTS[i] : SHORT_EVENTS[i];
                }
            }
        }
    }

    return BTN_NONE;
}
