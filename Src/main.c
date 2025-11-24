// main_quiz.c  ← 練習用の穴埋めバージョン
// 元の main.c を簡単にして、ところどころ ??? にしてあります。

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "config.h"
#include "hw_gpio.h"
#include "hw_i2c.h"
#include "hw_lcd.h"
#include "measure.h"

/* =========================================================================
 *  動作モード定義
 * =========================================================================*/
typedef enum { MODE_RES = 0, MODE_VOLT } mode_t;

/* --------------------------------------------------------------
 * set_mode_pins()
 *   抵抗 / 電圧モードで MOSFET & 分圧抵抗を切替
 * ------------------------------------------------------------*/
static void set_mode_pins(mode_t m)
{
    if (m == MODE_RES) {
        gpio_put(MOS_GP3, 1); gpio_put(MOS_GP2, 0);
        gpio_put(MOS_GP14,0); gpio_put(MOS_GP12,1);
    } else {
        gpio_put(MOS_GP3, 0); gpio_put(MOS_GP2, 1);
        gpio_put(MOS_GP14,1); gpio_put(MOS_GP12,0);
        /* 電圧モードは固定分圧 (10 kΩ) を選択 */
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
static void display_volt(int16_t mv)
{
    lcd_set_cursor(0);
    if (mv < 0) {
        lcd_write_char('-');
        mv = -mv;
    } else {
        lcd_write_char('+');
    }
    lcd_write_digits(mv/1000,2);
    lcd_write_char('.');
    lcd_write_digits((mv%1000)/10,2);
    lcd_write_char('V');
    lcd_write_char(' ');
}

/* ---- main() -------------------------------------------------------- */
int main(void)
{
    /* ★穴埋め1：UART(シリアル) と標準入出力の初期化
     *   Pico SDK の関数名を調べて、??? のところを書き換えてください。
     */
    ???();   // 例：標準入出力をまとめて初期化する関数

    sleep_ms(200);
    puts("UART ready");

    /* ★穴埋め2：各ハードウェアを初期化
     *   どの関数を呼んでいるかは、hw_gpio.h や measure.h を見るとわかるよ。
     */
    ???();   // GPIO の初期化
    ???();   // I2C の初期化
    ???();   // LCD の初期化
    ???();   // 測定(ADCまわり) の初期化

    /* 起動時にプッシュスイッチの状態でモード確定
     *   SW_VOL_GPIO が押されていたら 電圧モード
     *   そうでなければ 抵抗モード
     */
    /* ★穴埋め3：三項演算子 ( ? : ) を使って mode を決めている行です。
     *   MODE_VOLT と MODE_RES を使います。
     */
    mode_t mode = (gpio_get(SW_VOL_GPIO)==0) ? ??? : ???;

    set_mode_pins(mode);
    if (mode == MODE_RES) draw_res_fixed();

    while (true)
    {
        /* ---- スイッチでモード切替 -------------------------------- */
        mode_t new_mode = mode;

        /* ★穴埋め4：スイッチの状態で new_mode を変える
         *   SW_VOL_GPIO が 0 なら MODE_VOLT
         *   SW_RES_GPIO が 0 なら MODE_RES
         */
        if (gpio_get(SW_VOL_GPIO)==0)      new_mode = ???;
        else if (gpio_get(SW_RES_GPIO)==0) new_mode = ???;

        /* モードが変わったときの処理 */
        if (new_mode != mode) {
            mode = new_mode;
            set_mode_pins(mode);
            hw_led_blink(3,100);   // LED を3回ピカピカ

            /* LCD クリア */
            uint8_t clr[2] = {0x00,0x01};
            i2c_write_blocking(I2C_PORT, LCD_ADDR, clr, 2, false);
            sleep_ms(2);

            if (mode == MODE_RES) draw_res_fixed();
        }

        /* ---- メイン測定ループ ------------------------------------ */
        if (mode == MODE_RES) {
            /* ★穴埋め5：
             *   抵抗値を測る関数と、表示する関数を書いてみてください。
             *   ヒント：measure.h と、このファイルの上のほうに定義があります。
             */
            uint32_t ohm = ???(NULL);   // 抵抗値[Ω]を測定
            ???(ohm);                   // 抵抗値を LCD に表示
            hw_buzzer_set(ohm <= 80);   // 80Ω 以下ならブザー ON
        } else {
            /* ★穴埋め6：
             *   電圧を測る関数と、表示する関数。
             */
            int16_t mv = ???();         // 電圧[mV]を測定
            ???(mv);                    // 電圧を LCD に表示
            hw_buzzer_set(false);       // 電圧モードではブザー OFF
        }

        sleep_ms(50);                   // 50ms ごとに更新
    }

    return 0;
}
