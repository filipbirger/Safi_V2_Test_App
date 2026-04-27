#include "Particle.h"
#include "board.h"
#include "board_pin_defs.h"
#include "hal_selftest.h"
#include "../version.h"

// Global board state — 0xFF selftest_result signals "not yet run".
board_state_t g_board_state = {false, 0xFF, 0, FW_VERSION_STR};

// -----------------------------------------------------------------------------
// pins_init — configure all GPIO pins per board_pin_defs.h
//
// Only pins with confirmed Particle aliases are configured here.
// Commented-out blocks mark pins that are pending B524 pad-table verification
// (PIN_MOTPWM1/2, PIN_VMOTEN, PIN_BUZZER, PIN_LCD_RST, PIN_RGB_*, PIN_1WIRE,
// PIN_PWR_SHDN). Uncomment each block once the alias is confirmed.
// -----------------------------------------------------------------------------
void pins_init() {
    // LCD display control — CS deselected, backlight off, D/C idle-low
    pinMode(PIN_LCD_CS, OUTPUT);
    digitalWrite(PIN_LCD_CS, HIGH);     // deselect (active-low)
    pinMode(PIN_LCD_DC, OUTPUT);
    digitalWrite(PIN_LCD_DC, LOW);
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, LOW);      // backlight off until app enables it

    // LCD reset — TODO: uncomment once PIN_LCD_RST alias confirmed (P0.11)
    // pinMode(PIN_LCD_RST, OUTPUT);
    // digitalWrite(PIN_LCD_RST, HIGH);  // hold inactive (active-low)

    // Buttons — active-low inputs, pulled high internally
    pinMode(PIN_BTN_1, INPUT_PULLUP);
    pinMode(PIN_BTN_2, INPUT_PULLUP);
    // PIN_BTN_PWR / PIN_BTN_3: pin numbers not yet confirmed — add when known

    // PAC1951T power monitor
    pinMode(PIN_PAC_ALERT1, INPUT_PULLUP);  // open-drain interrupt, active-low
    pinMode(PIN_PAC_ALERT2, INPUT);          // ADC-capable alert / overcurrent flag
    pinMode(PIN_PAC_PWRDN, OUTPUT);
    digitalWrite(PIN_PAC_PWRDN, HIGH);      // keep PAC active (active-low power-down)

    // XC6192 power switch — SWOUT is an input wake signal
    pinMode(PIN_SWOUT, INPUT);

    // Motor driver — TODO: uncomment once P0.12 / P0.24 / P0.13 aliases confirmed
    // pinMode(PIN_MOTPWM1, OUTPUT); digitalWrite(PIN_MOTPWM1, LOW);
    // pinMode(PIN_MOTPWM2, OUTPUT); digitalWrite(PIN_MOTPWM2, LOW);
    // pinMode(PIN_VMOTEN,  OUTPUT); digitalWrite(PIN_VMOTEN,  LOW);

    // Buzzer — TODO: uncomment once P1.04 alias confirmed
    // pinMode(PIN_BUZZER, OUTPUT); digitalWrite(PIN_BUZZER, LOW);

    // RGB LED — TODO: uncomment once P0.14–P0.16 aliases confirmed
    // pinMode(PIN_RGB_R, OUTPUT); digitalWrite(PIN_RGB_R, LOW);
    // pinMode(PIN_RGB_G, OUTPUT); digitalWrite(PIN_RGB_G, LOW);
    // pinMode(PIN_RGB_B, OUTPUT); digitalWrite(PIN_RGB_B, LOW);

    // DS18B20 one-wire — TODO: uncomment once P0.07 alias confirmed
    // pinMode(PIN_1WIRE, INPUT);  // 1-Wire bus: external 4.7 kΩ pull-up on net

    // XC6192 power shutdown — TODO: uncomment once P0.02 safety verified on B524
    // pinMode(PIN_PWR_SHDN, OUTPUT); digitalWrite(PIN_PWR_SHDN, HIGH);
}

// -----------------------------------------------------------------------------
// board_init — top-level initialization sequence
//
// Step order is load-bearing: GPIO must precede bus init; UART must precede
// any log output; buses must be up before hal_selftest().
// -----------------------------------------------------------------------------
void board_init() {
    // 1. GPIO directions and pull configuration (must be first)
    pins_init();

    // 2. System clocks — ParticleOS configures the PLL and peripheral clocks
    //    automatically during early boot; no explicit call required here.

    // 3. Power rails — the B524 module manages its own LDOs. The XC6192 load
    //    switch (PIN_PWR_SHDN) can gate peripheral power; configure once
    //    PIN_PWR_SHDN is confirmed safe on the B524 pad table.

    // 4. Debug UART — physical header J502 (TX=P0.06, RX=P0.08 → Serial1).
    //    Must be up before any log output below.
    Serial1.begin(115200);
    delay(50);
    Serial1.printlnf("[BOOT] FW v%s", FW_VERSION_STR);

    // 5. I2C bus 0 @ 400 kHz fast-mode (MP2672AGD charger + PAC1951T monitor)
    Wire.setSpeed(400000);
    Wire.begin();

    // 6. SPI bus 0 @ 8 MHz, mode 0, MSB first (NV3007 LCD controller)
    SPI.begin();
    SPI.setClockSpeed(8000000);
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);

    // 7. Subsystem self-test — capture timestamp, run checks, record results
    g_board_state.boot_timestamp_ms = millis();
    g_board_state.fw_version        = FW_VERSION_STR;

    int result = hal_selftest();
    g_board_state.selftest_result = (uint8_t)result;

    if (result != HAL_SELFTEST_OK) {
        Serial1.printlnf("[BOOT] SELFTEST FAIL: 0x%02X", result);
        g_board_state.init_ok = false;
    } else {
        Serial1.println("[BOOT] All subsystems OK");
        g_board_state.init_ok = true;
    }
}
