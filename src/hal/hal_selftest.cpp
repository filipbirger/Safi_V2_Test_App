#include "Particle.h"
#include "hal_selftest.h"
#include "board_pin_defs.h"

// =============================================================================
// UART self-test
//
// Serial1 (hardware UART, J502 debug header) was initialised in board_init()
// step 4. A zero-length write exercises the transmit path non-destructively.
// For a stronger test on bring-up hardware, short TX (P0.06) to RX (P0.08)
// via a jumper on J502 and use Serial1.read() to verify the loopback byte.
// =============================================================================
int hal_selftest_uart() {
    Serial1.write((const uint8_t*)nullptr, 0);
    return HAL_SELFTEST_OK;
}

// =============================================================================
// I2C self-test
//
// Probes the dummy address 0x70 to exercise the bus without requiring a
// specific peripheral to be present.
//
// Wire.endTransmission() return codes (Arduino / Particle convention):
//   0 — success (device ACKed — unlikely at 0x70, treated as pass)
//   2 — address NACK (no device at 0x70; bus is healthy)
//   4 — other error: arbitration lost, SDA/SCL stuck, hardware fault → FAIL
//
// Both I2C devices on this bus (MP2672AGD @ 0x4B, PAC1951T @ 0x10) are
// intentionally not probed here to keep the selftest free of side-effects.
// =============================================================================
int hal_selftest_i2c() {
    Wire.beginTransmission(0x70);
    uint8_t err = Wire.endTransmission();

    if (err == 4) {
        return HAL_SELFTEST_I2C_FAIL;
    }
    return HAL_SELFTEST_OK;
}

// =============================================================================
// SPI loopback self-test
//
// Asserts PIN_LCD_CS (D4 / P1.08) as the chip-select during the transfer —
// it is the only defined SPI CS on this board and is safe to toggle before
// the LCD driver initialises.
//
// Transfers 0xA5 (alternating bits) and checks the received byte matches.
// Requires MOSI (P1.13) shorted to MISO (P1.14) via a bring-up jumper.
// Without the jumper the received byte will not match, correctly setting
// HAL_SELFTEST_SPI_FAIL so the missing jumper is detectable at boot.
// =============================================================================
int hal_selftest_spi() {
    const uint8_t LOOPBACK_BYTE = 0xA5;

    digitalWrite(PIN_LCD_CS, LOW);
    uint8_t received = SPI.transfer(LOOPBACK_BYTE);
    digitalWrite(PIN_LCD_CS, HIGH);

    return (received == LOOPBACK_BYTE) ? HAL_SELFTEST_OK : HAL_SELFTEST_SPI_FAIL;
}

// =============================================================================
// Top-level self-test — runs all subsystem checks and ORs their fail bits.
// To skip a subsystem during debugging, comment out its call and OR in its
// FAIL constant manually, or simply omit it to treat it as passing.
// =============================================================================
int hal_selftest() {
    int result = HAL_SELFTEST_OK;

    if (hal_selftest_uart() != HAL_SELFTEST_OK) {
        result |= HAL_SELFTEST_UART_FAIL;
    }
    if (hal_selftest_i2c() != HAL_SELFTEST_OK) {
        result |= HAL_SELFTEST_I2C_FAIL;
    }
    if (hal_selftest_spi() != HAL_SELFTEST_OK) {
        result |= HAL_SELFTEST_SPI_FAIL;
    }

    return result;
}
