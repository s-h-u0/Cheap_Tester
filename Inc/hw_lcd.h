#ifndef HW_LCD_H_
#define HW_LCD_H_
#include <stdint.h>
void hw_lcd_init(void);
void lcd_set_cursor(uint8_t pos);
void lcd_write_char(char c);
void lcd_write_digits(int val, uint8_t digits);
#endif