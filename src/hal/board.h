#pragma once

#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// board.h — top-level board state type and initialization API
// =============================================================================

// Runtime snapshot of board health. Written by board_init(); treat as
// read-only from all other modules after initialization completes.
typedef struct {
    bool        init_ok;            // true when all subsystems passed selftest
    uint8_t     selftest_result;    // raw bitmask returned by hal_selftest()
    uint32_t    boot_timestamp_ms;  // millis() captured at the start of selftest
    const char* fw_version;         // points to FW_VERSION_STR (never NULL)
} board_state_t;

// Global board state — populated once by board_init(), then read-only.
extern board_state_t g_board_state;

// Top-level board initialization sequence. Must be called once from setup()
// before any other subsystem code. Calls pins_init() internally.
void board_init();

// Configures all GPIO pins per board_pin_defs.h. Called as the first step
// of board_init(). May also be called standalone to re-apply pin config.
void pins_init();
