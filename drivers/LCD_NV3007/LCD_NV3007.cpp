// LCD driver for ER-TFT2.79-1 (142×428, NV3007 controller)
// Interface: 4-wire SPI  |  Platform: Particle DeviceOS (B524MEA / nRF52840)

#include "LCD_NV3007.h"
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
// 5×7 ASCII font (characters 0x20–0x7E)
// Each entry is 5 bytes, one byte per column, LSB = top pixel.
// ---------------------------------------------------------------------------
static const uint8_t s_font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // 0x20 space
    {0x00,0x00,0x5F,0x00,0x00}, // !
    {0x00,0x07,0x00,0x07,0x00}, // "
    {0x14,0x7F,0x14,0x7F,0x14}, // #
    {0x24,0x2A,0x7F,0x2A,0x12}, // $
    {0x23,0x13,0x08,0x64,0x62}, // %
    {0x36,0x49,0x55,0x22,0x50}, // &
    {0x00,0x05,0x03,0x00,0x00}, // '
    {0x00,0x1C,0x22,0x41,0x00}, // (
    {0x00,0x41,0x22,0x1C,0x00}, // )
    {0x08,0x2A,0x1C,0x2A,0x08}, // *
    {0x08,0x08,0x3E,0x08,0x08}, // +
    {0x00,0x50,0x30,0x00,0x00}, // ,
    {0x08,0x08,0x08,0x08,0x08}, // -
    {0x00,0x60,0x60,0x00,0x00}, // .
    {0x20,0x10,0x08,0x04,0x02}, // /
    {0x3E,0x51,0x49,0x45,0x3E}, // 0
    {0x00,0x42,0x7F,0x40,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46}, // 2
    {0x21,0x41,0x45,0x4B,0x31}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 6
    {0x01,0x71,0x09,0x05,0x03}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x06,0x49,0x49,0x29,0x1E}, // 9
    {0x00,0x36,0x36,0x00,0x00}, // :
    {0x00,0x56,0x36,0x00,0x00}, // ;
    {0x08,0x14,0x22,0x41,0x00}, // <
    {0x14,0x14,0x14,0x14,0x14}, // =
    {0x00,0x41,0x22,0x14,0x08}, // >
    {0x02,0x01,0x51,0x09,0x06}, // ?
    {0x32,0x49,0x79,0x41,0x3E}, // @
    {0x7E,0x11,0x11,0x11,0x7E}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x09,0x01}, // F
    {0x3E,0x41,0x49,0x49,0x7A}, // G
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x00,0x41,0x7F,0x41,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01}, // J
    {0x7F,0x08,0x14,0x22,0x41}, // K
    {0x7F,0x40,0x40,0x40,0x40}, // L
    {0x7F,0x02,0x04,0x02,0x7F}, // M
    {0x7F,0x04,0x08,0x10,0x7F}, // N
    {0x3E,0x41,0x41,0x41,0x3E}, // O
    {0x7F,0x09,0x09,0x09,0x06}, // P
    {0x3E,0x41,0x51,0x21,0x5E}, // Q
    {0x7F,0x09,0x19,0x29,0x46}, // R
    {0x46,0x49,0x49,0x49,0x31}, // S
    {0x01,0x01,0x7F,0x01,0x01}, // T
    {0x3F,0x40,0x40,0x40,0x3F}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x3F,0x40,0x38,0x40,0x3F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x07,0x08,0x70,0x08,0x07}, // Y
    {0x61,0x51,0x49,0x45,0x43}, // Z
    {0x00,0x7F,0x41,0x41,0x00}, // [
    {0x02,0x04,0x08,0x10,0x20}, // backslash
    {0x00,0x41,0x41,0x7F,0x00}, // ]
    {0x04,0x02,0x01,0x02,0x04}, // ^
    {0x40,0x40,0x40,0x40,0x40}, // _
    {0x00,0x01,0x02,0x04,0x00}, // `
    {0x20,0x54,0x54,0x54,0x78}, // a
    {0x7F,0x48,0x44,0x44,0x38}, // b
    {0x38,0x44,0x44,0x44,0x20}, // c
    {0x38,0x44,0x44,0x48,0x7F}, // d
    {0x38,0x54,0x54,0x54,0x18}, // e
    {0x08,0x7E,0x09,0x01,0x02}, // f
    {0x0C,0x52,0x52,0x52,0x3E}, // g
    {0x7F,0x08,0x04,0x04,0x78}, // h
    {0x00,0x44,0x7D,0x40,0x00}, // i
    {0x20,0x40,0x44,0x3D,0x00}, // j
    {0x7F,0x10,0x28,0x44,0x00}, // k
    {0x00,0x41,0x7F,0x40,0x00}, // l
    {0x7C,0x04,0x18,0x04,0x78}, // m
    {0x7C,0x08,0x04,0x04,0x78}, // n
    {0x38,0x44,0x44,0x44,0x38}, // o
    {0x7C,0x14,0x14,0x14,0x08}, // p
    {0x08,0x14,0x14,0x18,0x7C}, // q
    {0x7C,0x08,0x04,0x04,0x08}, // r
    {0x48,0x54,0x54,0x54,0x20}, // s
    {0x04,0x3F,0x44,0x40,0x20}, // t
    {0x3C,0x40,0x40,0x40,0x7C}, // u
    {0x1C,0x20,0x40,0x20,0x1C}, // v
    {0x3C,0x40,0x30,0x40,0x3C}, // w
    {0x44,0x28,0x10,0x28,0x44}, // x
    {0x0C,0x50,0x50,0x50,0x3C}, // y
    {0x44,0x64,0x54,0x4C,0x44}, // z
    {0x00,0x08,0x36,0x41,0x00}, // {
    {0x00,0x00,0x7F,0x00,0x00}, // |
    {0x00,0x41,0x36,0x08,0x00}, // }
    {0x08,0x08,0x2A,0x1C,0x08}, // ~
};

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static inline void cs_low(void)  { digitalWriteFast(LCD_PIN_CS, LOW);  }
static inline void cs_high(void) { digitalWriteFast(LCD_PIN_CS, HIGH); }
static inline void dc_cmd(void)  { digitalWriteFast(LCD_PIN_DC, LOW);  }
static inline void dc_data(void) { digitalWriteFast(LCD_PIN_DC, HIGH); }

static void write_cmd(uint8_t cmd)
{
    dc_cmd();
    cs_low();
    SPI.transfer(cmd);
    cs_high();
}

static void write_data8(uint8_t data)
{
    dc_data();
    cs_low();
    SPI.transfer(data);
    cs_high();
}

static void write_data16(uint16_t data)
{
    dc_data();
    cs_low();
    SPI.transfer((uint8_t)(data >> 8));
    SPI.transfer((uint8_t)(data & 0xFF));
    cs_high();
}

static void set_address_window(uint16_t x0, uint16_t y0,
                               uint16_t x1, uint16_t y1)
{
    write_cmd(NV3007_CASET);
    write_data16(x0);
    write_data16(x1);

    write_cmd(NV3007_RASET);
    write_data16(y0);
    write_data16(y1);

    write_cmd(NV3007_RAMWR);
}

// ---------------------------------------------------------------------------
// NV3007 power-on initialisation sequence
// ---------------------------------------------------------------------------
static void nv3007_init_sequence(void)
{
    // Hardware reset: hold low ≥10 ms, then release and wait ≥120 ms
    digitalWrite(LCD_PIN_RST, LOW);
    delay(15);
    digitalWrite(LCD_PIN_RST, HIGH);
    delay(120);

    write_cmd(NV3007_SWRESET);
    delay(120);

    write_cmd(NV3007_SLPOUT);
    delay(120);

    write_cmd(0xB1);            // Frame rate control (normal mode)
    write_data8(0x01);
    write_data8(0x2C);

    write_cmd(0xB2);            // Frame rate control (idle mode)
    write_data8(0x01);
    write_data8(0x2C);

    write_cmd(0xB3);            // Frame rate control (partial mode)
    write_data8(0x01);
    write_data8(0x2C);
    write_data8(0x01);
    write_data8(0x2C);

    write_cmd(0xB4);            // Display inversion: column inversion
    write_data8(0x07);

    write_cmd(0xC0);            // Power control 1
    write_data8(0xA2);
    write_data8(0x02);
    write_data8(0x84);

    write_cmd(0xC1);            // Power control 2
    write_data8(0xC5);

    write_cmd(0xC2);            // Power control 3 (normal mode)
    write_data8(0x0A);
    write_data8(0x00);

    write_cmd(0xC3);            // Power control 4 (idle mode)
    write_data8(0x8A);
    write_data8(0x2A);

    write_cmd(0xC4);            // Power control 5 (partial mode)
    write_data8(0x8A);
    write_data8(0xEE);

    write_cmd(0xC5);            // VCOM control
    write_data8(0x0E);

    write_cmd(NV3007_MADCTL);
    write_data8(MADCTL_RGB);    // portrait, RGB order

    write_cmd(NV3007_COLMOD);
    write_data8(COLMOD_16BIT);  // RGB565

    write_cmd(0xE0);            // Gamma (positive)
    write_data8(0x0F); write_data8(0x1A); write_data8(0x0F);
    write_data8(0x18); write_data8(0x2F); write_data8(0x28);
    write_data8(0x20); write_data8(0x22); write_data8(0x1F);
    write_data8(0x1B); write_data8(0x23); write_data8(0x37);
    write_data8(0x00); write_data8(0x07); write_data8(0x02);
    write_data8(0x10);

    write_cmd(0xE1);            // Gamma (negative)
    write_data8(0x0F); write_data8(0x1B); write_data8(0x0F);
    write_data8(0x17); write_data8(0x33); write_data8(0x2C);
    write_data8(0x29); write_data8(0x2E); write_data8(0x30);
    write_data8(0x30); write_data8(0x39); write_data8(0x3F);
    write_data8(0x00); write_data8(0x07); write_data8(0x03);
    write_data8(0x10);

    write_cmd(NV3007_NORON);
    delay(10);

    write_cmd(NV3007_DISPON);
    delay(100);
}

// ---------------------------------------------------------------------------
// Public API — core
// ---------------------------------------------------------------------------

bool LCD_Init(void)
{
    pinMode(LCD_PIN_CS,  OUTPUT);
    pinMode(LCD_PIN_DC,  OUTPUT);
    pinMode(LCD_PIN_RST, OUTPUT);
    pinMode(LCD_PIN_BL,  OUTPUT);

    cs_high();
    dc_data();
    digitalWrite(LCD_PIN_BL, LOW);

    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockSpeed(16, MHZ);

    nv3007_init_sequence();

    LCD_Clear(LCD_COLOR_BLACK);
    return true;
}

void LCD_SetBacklight(uint8_t brightness)
{
    analogWrite(LCD_PIN_BL, brightness);
}

void LCD_Clear(uint16_t colour)
{
    set_address_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    uint8_t hi = colour >> 8;
    uint8_t lo = colour & 0xFF;

    dc_data();
    cs_low();
    for (uint32_t i = 0; i < (uint32_t)LCD_WIDTH * LCD_HEIGHT; i++) {
        SPI.transfer(hi);
        SPI.transfer(lo);
    }
    cs_high();
}

void LCD_Sleep(void)
{
    write_cmd(NV3007_DISPOFF);
    delay(20);
    write_cmd(NV3007_SLPIN);
    delay(120);
}

void LCD_Wake(void)
{
    write_cmd(NV3007_SLPOUT);
    delay(120);
    write_cmd(NV3007_DISPON);
    delay(20);
}

// ---------------------------------------------------------------------------
// Public API — primitives
// ---------------------------------------------------------------------------

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t colour)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    set_address_window(x, y, x, y);
    write_data16(colour);
}

void LCD_FillRect(uint16_t x, uint16_t y,
                  uint16_t w, uint16_t h,
                  uint16_t colour)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH)  w = LCD_WIDTH  - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;

    set_address_window(x, y, x + w - 1, y + h - 1);

    uint8_t hi = colour >> 8;
    uint8_t lo = colour & 0xFF;

    dc_data();
    cs_low();
    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        SPI.transfer(hi);
        SPI.transfer(lo);
    }
    cs_high();
}

void LCD_DrawHLine(uint16_t x, uint16_t y, uint16_t len, uint16_t colour)
{
    LCD_FillRect(x, y, len, 1, colour);
}

void LCD_DrawVLine(uint16_t x, uint16_t y, uint16_t len, uint16_t colour)
{
    LCD_FillRect(x, y, 1, len, colour);
}

void LCD_DrawRect(uint16_t x, uint16_t y,
                  uint16_t w, uint16_t h,
                  uint16_t colour)
{
    LCD_DrawHLine(x,         y,         w, colour);
    LCD_DrawHLine(x,         y + h - 1, w, colour);
    LCD_DrawVLine(x,         y,         h, colour);
    LCD_DrawVLine(x + w - 1, y,         h, colour);
}

uint16_t LCD_DrawChar(uint16_t x, uint16_t y, char c,
                      uint16_t fg, uint16_t bg)
{
    if ((uint8_t)c < 0x20 || (uint8_t)c > 0x7E) c = '?';
    const uint8_t *glyph = s_font5x7[(uint8_t)c - 0x20];

    for (uint8_t col = 0; col < 5; col++) {
        uint8_t bits = glyph[col];
        for (uint8_t row = 0; row < 7; row++) {
            LCD_DrawPixel(x + col, y + row,
                          (bits & (1u << row)) ? fg : bg);
        }
    }
    LCD_DrawVLine(x + 5, y, 7, bg);  // inter-character gap
    return x + 6;
}

void LCD_DrawString(uint16_t x, uint16_t y, const char *str,
                    uint16_t fg, uint16_t bg)
{
    uint16_t cx = x;
    uint16_t cy = y;

    while (*str) {
        if (cx + 6 > LCD_WIDTH) {
            cx = x;
            cy += 9;            // 7px glyph + 2px leading
        }
        if (cy + 7 > LCD_HEIGHT) break;
        cx = LCD_DrawChar(cx, cy, *str, fg, bg);
        str++;
    }
}

// ---------------------------------------------------------------------------
// Public API — image display
// ---------------------------------------------------------------------------

void LCD_DrawImage(uint16_t x, uint16_t y,
                   uint16_t w, uint16_t h,
                   const uint16_t *pixels)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH)  w = LCD_WIDTH  - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;

    set_address_window(x, y, x + w - 1, y + h - 1);

    dc_data();
    cs_low();

    // LVGL converter with "Swap bytes" ON outputs pixels in the correct
    // big-endian byte order for SPI.  Send each uint16_t as two raw bytes.
    uint32_t total = (uint32_t)w * h;
    for (uint32_t i = 0; i < total; i++) {
        SPI.transfer((uint8_t)(pixels[i] >> 8));
        SPI.transfer((uint8_t)(pixels[i] & 0xFF));
    }

    cs_high();
}

void LCD_DrawStaticUI(const uint16_t *pixels)
{
    // Image must be LCD_WIDTH × LCD_REGION_STATIC_H pixels
    LCD_DrawImage(0, LCD_REGION_STATIC_Y, LCD_WIDTH, LCD_REGION_STATIC_H, pixels);
}

void LCD_DrawStateButtons(const uint16_t *pixels)
{
    // Image must be LCD_WIDTH × LCD_REGION_STATE_H pixels
    LCD_DrawImage(0, LCD_REGION_STATE_Y, LCD_WIDTH, LCD_REGION_STATE_H, pixels);
}

// ---------------------------------------------------------------------------
// Public API — dynamic status elements
// ---------------------------------------------------------------------------

void LCD_DrawBatteryLevel(uint8_t percent,
                          uint16_t fill_colour,
                          uint16_t bg_colour)
{
    if (percent > 100) percent = 100;

    const uint16_t bx = LCD_BATT_X;
    const uint16_t by = LCD_BATT_Y;

    // Battery body dimensions (leave 3px on right for the terminal nub)
    const uint16_t body_w = LCD_BATT_W - 3;
    const uint16_t body_h = LCD_BATT_H;

    // Outer outline
    LCD_DrawRect(bx, by, body_w, body_h, fill_colour);

    // Terminal nub (right side, centred vertically)
    LCD_FillRect(bx + body_w, by + (body_h / 2) - 2, 3, 4, fill_colour);

    // Inner fill area
    const uint16_t inner_x = bx + 2;
    const uint16_t inner_y = by + 2;
    const uint16_t inner_w = body_w - 4;
    const uint16_t inner_h = body_h - 4;

    // Erase full inner area first
    LCD_FillRect(inner_x, inner_y, inner_w, inner_h, bg_colour);

    // Fill proportional bar
    uint16_t bar_w = (uint16_t)((uint32_t)inner_w * percent / 100);
    if (bar_w > 0) {
        LCD_FillRect(inner_x, inner_y, bar_w, inner_h, fill_colour);
    }
}

void LCD_UpdateTemperature(float tempC, uint16_t fg_colour, uint16_t bg_colour)
{
    // Erase the entire text field first so shorter strings don't leave stale
    // pixels (e.g. "100.0 C" → "9.5 C")
    LCD_FillRect(LCD_TEMP_X, LCD_TEMP_Y, LCD_TEMP_FIELD_W, 7, bg_colour);

    char buf[12];
    snprintf(buf, sizeof(buf), "%.1f C", tempC);
    LCD_DrawString(LCD_TEMP_X, LCD_TEMP_Y, buf, fg_colour, bg_colour);
}
