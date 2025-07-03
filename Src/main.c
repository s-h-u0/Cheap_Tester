#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "config.h"
#include "hw_gpio.h"
#include "hw_i2c.h"
#include "hw_lcd.h"
#include "measure.h"

/* ---- 電圧変換定数 -------------------------------------------------- */
#define RES       0.0625f   /* mV/LSB (16‑bit, PGA×1) */
#define ATT_ADC1  11        /* 分圧比                 */
#define G_CALIB   0.998f    /* 校正係数               */

/* ---- 動作モード ---------------------------------------------------- */
typedef enum { MODE_RES = 0, MODE_VOLT } mode_t;

/* --------------------------------------------------------------
 * set_mode_pins()
 *   抵抗 / 電圧モードで MOSFET & 分圧抵抗を切替
 * ------------------------------------------------------------ */
static void set_mode_pins(mode_t m)
{
    if (m == MODE_RES) {
        gpio_put(MOS_GP3, 1); gpio_put(MOS_GP2, 0);
        gpio_put(MOS_GP14,0); gpio_put(MOS_GP12,1);
    } else {
        gpio_put(MOS_GP3, 0); gpio_put(MOS_GP2, 1);
        gpio_put(MOS_GP14,1); gpio_put(MOS_GP12,0);
        /* 電圧モードは固定分圧 (10 kΩ) を選択 */
        gpio_put(R1_GPIO,0);  gpio_put(R2_GPIO,1);
        gpio_put(R3_GPIO,0);  gpio_put(R4_GPIO,0);
    }
}

/* ---- 抵抗モード固定表示 "kΩ." ---------------------------------- */
static void draw_res_fixed(void)
{
    lcd_set_cursor(4);        lcd_write_char('.');
    lcd_set_cursor(0x40+6);   lcd_write_char('k');
    lcd_set_cursor(0x40+7);   lcd_write_char(30); /* Ω 記号 */
}

/* ---- 抵抗値表示 ---------------------------------------------------- */
static void display_res(uint32_t ohm)
{
    lcd_set_cursor(0); lcd_write_digits(ohm/1000,4);
    lcd_set_cursor(5); lcd_write_digits(ohm%1000,3);
}

/* ---- 電圧表示 "±xx.xxV" ------------------------------------------- */
static void display_volt(int16_t raw)
{
    float mv_f = raw * (ATT_ADC1 * RES) * G_CALIB; /* mV (float) */
    int16_t mv = (int16_t)mv_f;                    /* mV (int)   */

    lcd_set_cursor(0);
    if (raw < 0) { lcd_write_char('-'); mv = -mv; }
    else         { lcd_write_char('+');           }

    lcd_write_digits(mv/1000,2);
    lcd_write_char('.');
    lcd_write_digits((mv%1000)/10,2);
    lcd_write_char('V');
    lcd_write_char(' ');
}

/* ---- main() -------------------------------------------------------- */
int main(void)
{
    stdio_init_all();
    sleep_ms(200);
    puts("UART ready");

    hw_gpio_init();
    hw_i2c_init();
    hw_lcd_init();
    measure_init();

    mode_t mode = (gpio_get(SW_VOL_GPIO)==0) ? MODE_VOLT : MODE_RES;
    set_mode_pins(mode);
    if (mode == MODE_RES) draw_res_fixed();

    while (true)
    {
        /* --- スイッチでモード切替 ---------------------------------- */
        mode_t new_mode = mode;
        if (gpio_get(SW_VOL_GPIO)==0)      new_mode = MODE_VOLT;
        else if (gpio_get(SW_RES_GPIO)==0) new_mode = MODE_RES;

        if (new_mode != mode) {
            mode = new_mode;
            set_mode_pins(mode);
            hw_led_blink(3,100);
            /* LCD クリア */
            uint8_t clr[2] = {0x00,0x01};
            i2c_write_blocking(I2C_PORT, LCD_ADDR, clr, 2, false);
            sleep_ms(2);
            if (mode == MODE_RES) draw_res_fixed();
        }

        /* --- メイン測定ループ -------------------------------------- */
        if (mode == MODE_RES) {
            uint32_t ohm = measure_resistance(NULL);
            display_res(ohm);
            hw_buzzer_set(ohm <= 80);
        } else {
            int16_t raw = measure_voltage();
            display_volt(raw);
            hw_buzzer_set(false);
        }
        sleep_ms(50);
    }
    return 0;
}
