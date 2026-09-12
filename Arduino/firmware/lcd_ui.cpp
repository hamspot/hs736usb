#include "lcd_ui.h"

#include <Arduino.h>
#include <LiquidCrystal.h>

#include "cat_map.h"

/* RS D11, E D12, 4-bit data A2–A5. RW tied to GND. */
static LiquidCrystal s_lcd(11, 12, A2, A3, A4, A5);

static uint32_t s_last_ms;
static uint8_t s_last_pwm;

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
    /* 8 packed BCD digits, 10 Hz units → ddd.dddd */
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

void lcd_ui_begin(void)
{
    s_lcd.begin(16, 2);
    s_lcd.clear();
    s_lcd.print("HS-736USB");
    s_lcd.setCursor(0, 1);
    s_lcd.print("CAT wait");
    s_last_ms = 0;
    s_last_pwm = 0xFF;
}

void lcd_ui_poll(uint32_t now_ms, uint8_t pwm)
{
    uint8_t mainb[5];
    uint8_t radio[5];
    char line[17];
    const char *md;
    uint8_t bars;
    uint8_t i;

    if (s_last_ms != 0 && (uint32_t)(now_ms - s_last_ms) < 250u && pwm == s_last_pwm) {
        return;
    }
    s_last_ms = now_ms;
    s_last_pwm = pwm;

    cat_copy_main(mainb);
    cat_copy_last_radio(radio);
    fmt_freq(mainb, line);
    md = mode_text(mainb[4]);
    s_lcd.setCursor(0, 0);
    s_lcd.print(line);
    s_lcd.print(' ');
    s_lcd.print(md);
    s_lcd.print(cat_map_proto() == CAT_PROTO_847 ? " 847" : " 736");

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
    s_lcd.setCursor(0, 1);
    s_lcd.print(line);
}
