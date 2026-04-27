#pragma once

#include <stdint.h>

// =============================================================================
// hal_selftest.h — HAL self-test API and subsystem bitmask constants
//
// hal_selftest() return value encodes each failing subsystem as a set bit:
//
//   Bit 0  (0x01)  UART  — HAL_SELFTEST_UART_FAIL
//   Bit 1  (0x02)  I2C   — HAL_SELFTEST_I2C_FAIL
//   Bit 2  (0x04)  SPI   — HAL_SELFTEST_SPI_FAIL
//   Bits 3–7        reserved for future subsystems
//
// When adding a new subsystem: define its bit constant here AND document it
// in hal_selftest.cpp. Do not reuse or reorder existing bit positions.
// =============================================================================

#define HAL_SELFTEST_OK         0x00u
#define HAL_SELFTEST_UART_FAIL  (1u << 0)   // 0x01
#define HAL_SELFTEST_I2C_FAIL   (1u << 1)   // 0x02
#define HAL_SELFTEST_SPI_FAIL   (1u << 2)   // 0x04

// Run all subsystem checks. Returns HAL_SELFTEST_OK (0) on full success;
// non-zero bitmask encodes every failing subsystem.
int hal_selftest();

// Individual subsystem tests — callable independently for targeted debugging.
// Each returns HAL_SELFTEST_OK on pass or its own FAIL constant on failure.
int hal_selftest_uart();
int hal_selftest_i2c();
int hal_selftest_spi();
