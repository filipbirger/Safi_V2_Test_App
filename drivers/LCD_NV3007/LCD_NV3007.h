#ifndef LCD_NV3007_H
#define LCD_NV3007_H

#include <stdint.h>
#include <stdbool.h>
#include "Particle.h"

// ---------------------------------------------------------------------------
// Hardware pins  — update when board pin map header (story #2) is defined
// Project schematic: CS=P1.08, RS(D/C)=P1.09, RSTB=P0.11, BL_PWM=P1.01
// ---------------------------------------------------------------------------
#define LCD_PIN_CS      D8      // P1.08 — chip select, active-low
#define LCD_PIN_DC      D9      // P1.09 — data/command: LOW=cmd, HIGH=data
#define LCD_PIN_RST     D6      // P0.11 — hardware reset, active-low
#define LCD_PIN_BL      D4      // P1.01 — backlight PWM (BSS138PW gate)

// ---------------------------------------------------------------------------
// Display geometry
// ---------------------------------------------------------------------------
#define LCD_WIDTH       142
#define LCD_HEIGHT      428

// ---------------------------------------------------------------------------
// Screen layout — two non-overlapping regions painted independently.
//
//   ┌──────────────┐  y=0
//   │              │
//   │  STATIC UI   │  LCD_REGION_STATIC_H rows
//   │  (drawn once)│  Contains status bar (battery + temperature)
//   │              │
//   ├──────────────┤  y=LCD_REGION_STATE_Y
//   │  STATE IMAGE │  LCD_REGION_STATE_H rows
//   │ (per-state)  │  Updated on every state transition
//   └──────────────┘  y=LCD_HEIGHT-1
//
// Adjust LCD_REGION_STATIC_H to match your actual artwork split.
// ---------------------------------------------------------------------------
#define LCD_REGION_STATIC_H     300
#define LCD_REGION_STATE_Y      LCD_REGION_STATIC_H
#define LCD_REGION_STATE_H      (LCD_HEIGHT - LCD_REGION_STATIC_H)  // 128

// Status-bar elements live inside the static region.
// Adjust x/y to match your artwork layout.
#define LCD_TEMP_X              8       // top-left of temperature text field
#define LCD_TEMP_Y              8
#define LCD_TEMP_FIELD_W        72      // pixel width to erase before redraw
                                        // (must be wide enough for "100.0 C")
#define LCD_BATT_X              104     // top-left of battery indicator
#define LCD_BATT_Y              8
#define LCD_BATT_W              30      // total width  (body + tip)
#define LCD_BATT_H              14      // total height

// ---------------------------------------------------------------------------
// Image arrays produced by the LVGL online image converter
//   Settings: Colour format → True colour (16-bit), Output format → C array,
//             Swap bytes (big-endian) → YES (to match SPI byte order).
// Declare your arrays like:
//   extern const uint16_t img_idle[];
//   extern const uint16_t img_preheat[];
//   … etc.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// NV3007 command codes
// ---------------------------------------------------------------------------
#define NV3007_NOP          0x00
#define NV3007_SWRESET      0x01
#define NV3007_SLPIN        0x10
#define NV3007_SLPOUT       0x11
#define NV3007_PTLON        0x12
#define NV3007_NORON        0x13
#define NV3007_INVOFF       0x20
#define NV3007_INVON        0x21
#define NV3007_DISPOFF      0x28
#define NV3007_DISPON       0x29
#define NV3007_CASET        0x2A
#define NV3007_RASET        0x2B
#define NV3007_RAMWR        0x2C
#define NV3007_MADCTL       0x36
#define NV3007_COLMOD       0x3A

#define MADCTL_MY   0x80
#define MADCTL_MX   0x40
#define MADCTL_MV   0x20
#define MADCTL_ML   0x10
#define MADCTL_RGB  0x00
#define MADCTL_BGR  0x08
#define COLMOD_16BIT 0x55

// ---------------------------------------------------------------------------
// RGB-565 colour helpers
// ---------------------------------------------------------------------------
#define LCD_COLOR_BLACK     0x0000u
#define LCD_COLOR_WHITE     0xFFFFu
#define LCD_COLOR_RED       0xF800u
#define LCD_COLOR_GREEN     0x07E0u
#define LCD_COLOR_BLUE      0x001Fu
#define LCD_COLOR_YELLOW    0xFFE0u
#define LCD_COLOR_CYAN      0x07FFu
#define LCD_COLOR_MAGENTA   0xF81Fu

#define LCD_RGB(r, g, b) \
    ((uint16_t)(((uint16_t)((r) & 0xF8) << 8) | \
                ((uint16_t)((g) & 0xFC) << 3) | \
                ((uint16_t)((b) & 0xF8) >> 3)))

// ---------------------------------------------------------------------------
// Public API — core
// ---------------------------------------------------------------------------

// Initialise SPI, GPIO, and send the NV3007 power-on sequence.
bool LCD_Init(void);

// Set backlight brightness 0–255 (0 = off, 255 = full).
void LCD_SetBacklight(uint8_t brightness);

// Fill the entire screen with one colour.
void LCD_Clear(uint16_t colour);

// Sleep / wake the controller (backlight should be off before sleep).
void LCD_Sleep(void);
void LCD_Wake(void);

// ---------------------------------------------------------------------------
// Public API — primitives
// ---------------------------------------------------------------------------
void     LCD_DrawPixel  (uint16_t x, uint16_t y, uint16_t colour);
void     LCD_FillRect   (uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour);
void     LCD_DrawHLine  (uint16_t x, uint16_t y, uint16_t len, uint16_t colour);
void     LCD_DrawVLine  (uint16_t x, uint16_t y, uint16_t len, uint16_t colour);
void     LCD_DrawRect   (uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour);
uint16_t LCD_DrawChar   (uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg);
void     LCD_DrawString (uint16_t x, uint16_t y, const char *str, uint16_t fg, uint16_t bg);

// ---------------------------------------------------------------------------
// Public API — image display
//
// LCD_DrawImage() is the core blit. Pass any LVGL-converted uint16_t array.
// w and h must match the pixel dimensions of the converted image exactly.
// ---------------------------------------------------------------------------
void LCD_DrawImage(uint16_t x, uint16_t y,
                   uint16_t w, uint16_t h,
                   const uint16_t *pixels);

// Convenience wrappers that use the pre-defined layout regions above.
// Pass the LVGL array for the static artwork (142 × LCD_REGION_STATIC_H px).
void LCD_DrawStaticUI(const uint16_t *pixels);

// Pass the LVGL array for the current state's button strip
// (142 × LCD_REGION_STATE_H px).  Call on every state transition.
void LCD_DrawStateButtons(const uint16_t *pixels);

// ---------------------------------------------------------------------------
// Public API — dynamic status elements
//
// These target the small sub-regions inside the static UI area.
// Call them whenever the underlying value changes — they do a
// minimal erase + redraw so there is no visible flicker.
// ---------------------------------------------------------------------------

// Draw a battery icon at (LCD_BATT_X, LCD_BATT_Y).
// percent: 0–100.  fill_colour is the bar colour (e.g. green/yellow/red).
// bg_colour must match the artwork background at that position.
void LCD_DrawBatteryLevel(uint8_t percent,
                          uint16_t fill_colour,
                          uint16_t bg_colour);

// Erase the temperature field and redraw with the new value.
// tempC is written as "XX.X C" using the built-in font.
// bg_colour must match the artwork background at that position.
void LCD_UpdateTemperature(float tempC, uint16_t fg_colour, uint16_t bg_colour);

#endif // LCD_NV3007_H
