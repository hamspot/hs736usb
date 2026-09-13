#ifndef LCD_UI_H
#define LCD_UI_H

#include <stdint.h>

void lcd_ui_begin(void);
void lcd_ui_poll(uint32_t now_ms, uint8_t pwm);
void lcd_ui_set_cursor(uint8_t nibble);
uint8_t lcd_ui_cursor(void);

#endif
