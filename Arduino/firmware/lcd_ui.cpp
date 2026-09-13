#include "lcd_ui.h"
#include "hs736_features.h"

#if HS736_USE_MCP23017

#include "cat_map.h"
#include "encoder_logic.h"
#include "mcp23017.h"

#ifdef ARDUINO
#include <Arduino.h>
#define LCD_DELAY_US(x) delayMicroseconds(x)
#define LCD_DELAY_MS(x) delay(x)
#else
#define LCD_DELAY_US(x) ((void)(x))
#define LCD_DELAY_MS(x) ((void)(x))
#endif

static uint32_t s_last_ms;
static uint8_t s_last_pwm;
static uint8_t s_cursor = ENC_CURSOR_DEFAULT;
static uint8_t s_olat;

static char nibble_hex(uint8_t v)
{
    v = (uint8_t)(v & 0x0Fu);
    return (char)((v < 10) ? ('0' + v) : ('A' + (v - 10)));
}

static const char *mode_text(uint8_t m)
{
    switch (m) {
    case 0x00:
        return "LSB";
    case 0x01:
        return "USB";
    case 0x02:
        return "CW ";
    case 0x03:
        return "CWR";
    case 0x04:
        return "AM ";
    case 0x08:
        return "FM ";
    case 0x82:
        return "CWN";
    case 0x88:
        return "FMN";
    default:
        return "???";
    }
}

static void fmt_freq(const uint8_t *bcd, char *dst)
{
    dst[0] = (char)('0' + ((bcd[0] >> 4) & 0x0F));
    dst[1] = (char)('0' + (bcd[0] & 0x0F));
    dst[2] = (char)('0' + ((bcd[1] >> 4) & 0x0F));
    dst[3] = '.';
    dst[4] = (char)('0' + (bcd[1] & 0x0F));
    dst[5] = (char)('0' + ((bcd[2] >> 4) & 0x0F));
    dst[6] = (char)('0' + (bcd[2] & 0x0F));
    dst[7] = (char)('0' + ((bcd[3] >> 4) & 0x0F));
    dst[8] = '\0';
}

static void lcd_pulse(uint8_t data_rs)
{
    mcp23017_write_a((uint8_t)(data_rs | MCP_LCD_E));
    LCD_DELAY_US(2);
    mcp23017_write_a(data_rs);
    LCD_DELAY_US(40);
    s_olat = data_rs;
}

static void lcd_nibble(uint8_t nib, uint8_t rs)
{
    uint8_t v;

    v = (uint8_t)((nib & 0x0Fu) | (rs ? MCP_LCD_RS : 0));
    lcd_pulse(v);
}

static void lcd_byte(uint8_t b, uint8_t rs)
{
    lcd_nibble((uint8_t)(b >> 4), rs);
    lcd_nibble((uint8_t)(b & 0x0Fu), rs);
}

static void lcd_cmd(uint8_t b)
{
    lcd_byte(b, 0);
    if (b == 0x01 || b == 0x02) {
        LCD_DELAY_MS(2);
    }
}

static void lcd_data(uint8_t b)
{
    lcd_byte(b, 1);
}

static void lcd_puts(const char *s)
{
    while (*s) {
        lcd_data((uint8_t)*s++);
    }
}

static void lcd_set_ddram(uint8_t addr)
{
    lcd_cmd((uint8_t)(0x80u | addr));
}

void lcd_ui_set_cursor(uint8_t nibble)
{
    s_cursor = nibble;
    s_last_ms = 0;
}

uint8_t lcd_ui_cursor(void)
{
    return s_cursor;
}

void lcd_ui_begin(void)
{
    (void)mcp23017_begin();
    LCD_DELAY_MS(50);
    lcd_nibble(0x03, 0);
    LCD_DELAY_MS(5);
    lcd_nibble(0x03, 0);
    LCD_DELAY_US(150);
    lcd_nibble(0x03, 0);
    lcd_nibble(0x02, 0);
    lcd_cmd(0x28);
    lcd_cmd(0x08);
    lcd_cmd(0x01);
    lcd_cmd(0x06);
    lcd_cmd(0x0C);
    lcd_set_ddram(0);
    lcd_puts("HS-736USB");
    lcd_set_ddram(0x40);
    lcd_puts("CAT wait");
    s_last_ms = 0;
    s_last_pwm = 0xFF;
    s_cursor = ENC_CURSOR_DEFAULT;
}

void lcd_ui_poll(uint32_t now_ms, uint8_t pwm)
{
    uint8_t mainb[5];
    uint8_t radio[5];
    char line[17];
    const char *md;
    uint8_t bars;
    uint8_t i;
    uint8_t col;

    if (s_last_ms != 0 && (uint32_t)(now_ms - s_last_ms) < 250u && pwm == s_last_pwm) {
        return;
    }
    s_last_ms = now_ms;
    s_last_pwm = pwm;

    cat_copy_main(mainb);
    cat_copy_last_radio(radio);
    fmt_freq(mainb, line);
    md = mode_text(mainb[4]);
    lcd_set_ddram(0);
    lcd_puts(line);
    lcd_data(' ');
    lcd_puts(md);
    lcd_puts(cat_map_proto() == CAT_PROTO_847 ? " 847" : " 736");

    for (i = 0; i < 5; i++) {
        line[i * 2] = nibble_hex((uint8_t)(radio[i] >> 4));
        line[i * 2 + 1] = nibble_hex(radio[i]);
    }
    line[10] = ' ';
    bars = (uint8_t)((uint16_t)pwm * 5u / 255u);
    if (pwm > 0 && bars == 0) {
        bars = 1;
    }
    if (pwm == 255) {
        bars = 5;
    }
    for (i = 0; i < 5; i++) {
        line[11 + i] = (i < bars) ? '#' : ' ';
    }
    line[16] = '\0';
    lcd_set_ddram(0x40);
    lcd_puts(line);

    if (s_cursor == ENC_CURSOR_DEFAULT) {
        lcd_cmd(0x0C);
    } else {
        col = s_cursor;
        if (col >= 3) {
            col = (uint8_t)(col + 1u);
        }
        lcd_cmd(0x0E);
        lcd_set_ddram(col);
    }
}

#else /* !HS736_USE_MCP23017 */

void lcd_ui_begin(void) {}
void lcd_ui_poll(uint32_t now_ms, uint8_t pwm)
{
    (void)now_ms;
    (void)pwm;
}
void lcd_ui_set_cursor(uint8_t nibble)
{
    (void)nibble;
}
uint8_t lcd_ui_cursor(void)
{
    return 0xFF;
}

#endif
