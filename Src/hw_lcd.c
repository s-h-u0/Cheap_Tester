#include "hw_lcd.h"
#include "config.h"
#include "hardware/i2c.h"

/* ---- 内部ヘルパ ------------------------------------------------------ */
static inline void _cmd(uint8_t c)
{
    uint8_t d[2] = { 0x00, c };
    i2c_write_blocking(I2C_PORT, LCD_ADDR, d, 2, false);
}

void hw_lcd_init(void)
{
    const uint8_t seq[] = {
        0x38, 0x39, 0x14, 0x70, 0x56,
        0x6C,             /* booster on */
        0x38, 0x0C, 0x01  /* normal mode */
    };
    for (uint i = 0; i < sizeof seq; ++i) {
        _cmd(seq[i]);
        sleep_ms((i == 5) ? 200 : (i == 8) ? 2 : 0);
    }
}

void lcd_set_cursor(uint8_t pos)
{
    _cmd(0x80 | pos);
}

void lcd_write_char(char c)
{
    uint8_t d[2] = { 0x40, (uint8_t)c };
    i2c_write_blocking(I2C_PORT, LCD_ADDR, d, 2, false);
}

void lcd_write_digits(int val, uint8_t digits)
{
    char buf[4] = { '0', '0', '0', '0' };
    for (int i = 0; i < 4; ++i) {
        buf[3 - i] = '0' + (val % 10);
        val /= 10;
    }
    for (uint8_t i = 0; i < digits; ++i) {
        lcd_write_char(buf[4 - digits + i]);
    }
}
