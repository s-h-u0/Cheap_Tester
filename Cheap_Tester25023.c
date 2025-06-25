/*
 * Multi-mode Tester  (Resistance / Voltage)
 * 2025-06-25  Shu Morishima
 *
 *  - 抵抗モード : 自動レンジ + kΩ表示 + ブザー (<80 Ω)
 *  - 電圧モード : ±xx.xx V 表示
 *  - GP17 = 抵抗モード SW (Low 有効)
 *    GP18 = 電圧モード SW (Low 有効)
 *  - 切替時に LED(GP25) を 3 回点滅
 */


#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"

// ── GPIO ───────────────────────────────────
#define R1_GPIO   13
#define R2_GPIO   10
#define R3_GPIO    8
#define R4_GPIO    6

#define MOS_GP3    3
#define MOS_GP2    2
#define MOS_GP14  14
#define MOS_GP12  12

#define BUZZER_GPIO 5
#define LED_GPIO   25

#define SW_RES_GPIO 17
#define SW_VOL_GPIO 18

// ── I2C & アドレス ─────────────────────────
#define I2C_PORT i2c1
#define PIN_SDA  26
#define PIN_SCL  27
#define LCD_ADDR 0x3E     /* AQM0802A */
#define ADC_ADDR 0x68     /* MCP3425 */

// ── 抵抗測定定数 ────────────────────────────
#define RESOLUTION_FACTOR (2.048f / 32768.0f * 1000.0f) /* mV/LSB */
static const float gain_table[4] = {0.986f, 0.975f, 1.000f, 0.668f};

// ── 電圧測定定数 ────────────────────────────
#define RES      0.0625f  /* mV/LSB (16 bit, PGA×1) */
#define ATT_ADC1 11       /* 分圧比 */
#define G_CALIB  0.998f   /* 校正係数 */

// ── 型 ──────────────────────────────────────
typedef enum { MODE_RESISTANCE = 0, MODE_VOLTAGE } meas_mode_t;

// ── プロトタイプ ────────────────────────────
static void init_gpio(void);
static void init_i2c(void);
static void init_adc(void);
static void init_lcd(void);
static void init_buzzer(void);

static void set_mode_pins(meas_mode_t mode);
static void blink_led(uint8_t times, uint32_t ms);

static int16_t read_adc_raw(void);

static void draw_res_fixed(void);
static void lcd_set_cursor(uint8_t pos);
static void lcd_write_char(char c);
static void lcd_write_digits(int val, uint8_t d);

static void display_resistance(uint32_t dut);
static void display_voltage(int16_t raw);

static void range_sw(uint8_t ch);

// ── main ───────────────────────────────────
int main(void)
{
    stdio_init_all();
    sleep_ms(200);

    printf("UART console ready\r\n");

    init_gpio();
    init_buzzer();
    init_i2c();
    init_adc();
    init_lcd();

    meas_mode_t mode = (gpio_get(SW_VOL_GPIO)==0)
                       ? MODE_VOLTAGE : MODE_RESISTANCE;
    set_mode_pins(mode);
    if (mode == MODE_RESISTANCE) draw_res_fixed();

    uint8_t range_ch = 0;

    while (true)
    {
        /* スイッチでモード変更 */
        meas_mode_t new_mode = mode;
        if (gpio_get(SW_VOL_GPIO)==0) new_mode = MODE_VOLTAGE;
        else if (gpio_get(SW_RES_GPIO)==0) new_mode = MODE_RESISTANCE;

        if (new_mode != mode) {
            mode = new_mode;
            set_mode_pins(mode);
            blink_led(3,100);

            uint8_t clr[2]={0x00,0x01};
            i2c_write_blocking(I2C_PORT,LCD_ADDR,clr,2,false);
            sleep_ms(2);
            if (mode==MODE_RESISTANCE) draw_res_fixed();
        }

        /* モード別メインループ */
        if (mode == MODE_RESISTANCE) {
            float val_mv;
            while (true) {
                range_sw(range_ch);
                sleep_ms(100);
                val_mv = read_adc_raw() * RESOLUTION_FACTOR;
                if ((val_mv>500 && val_mv<1500) ||
                    (val_mv<=500 && range_ch==3) ||
                    (val_mv>=1500 && range_ch==0))
                    break;
                if (val_mv<=500) range_ch++; else range_ch--;
            }

            float dut_f;
            if      (range_ch==0) dut_f =   1000.0f*(2048.0f-val_mv)/val_mv;
            else if (range_ch==1) dut_f =  10000.0f*(2048.0f-val_mv)/val_mv;
            else if (range_ch==2) dut_f = 100000.0f*(2048.0f-val_mv)/val_mv;
            else                  dut_f =1000000.0f*(2048.0f-val_mv)/val_mv;
            dut_f *= gain_table[range_ch];
            uint32_t dut = (uint32_t)(dut_f + 0.5f);

            display_resistance(dut);
            pwm_set_enabled(pwm_gpio_to_slice_num(BUZZER_GPIO),(dut<=80));
        }
        else {                      /* MODE_VOLTAGE */
            int16_t raw = read_adc_raw();
            display_voltage(raw);
            pwm_set_enabled(pwm_gpio_to_slice_num(BUZZER_GPIO),false);
        }
        sleep_ms(50);
    }
    return 0;
}

// ── 初期化関数 ───────────────────────────────
static void init_gpio(void)
{
    uint16_t outs[] = {R1_GPIO,R2_GPIO,R3_GPIO,R4_GPIO,
                       MOS_GP3,MOS_GP2,MOS_GP14,MOS_GP12};
    for (uint i=0;i<sizeof outs/sizeof outs[0];++i){
        gpio_init(outs[i]); gpio_set_dir(outs[i],GPIO_OUT); gpio_put(outs[i],0);
    }
    uint16_t ins[] = {SW_RES_GPIO,SW_VOL_GPIO};
    for (uint i=0;i<sizeof ins/sizeof ins[0];++i){
        gpio_init(ins[i]); gpio_set_dir(ins[i],GPIO_IN); gpio_pull_up(ins[i]);
    }
    gpio_init(LED_GPIO); gpio_set_dir(LED_GPIO,GPIO_OUT);
}
static void init_i2c(void)
{
    i2c_init(I2C_PORT,100*1000);
    gpio_set_function(PIN_SDA,GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL,GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA); gpio_pull_up(PIN_SCL);
    sleep_ms(200);
}
static void init_adc(void)
{
    uint8_t cfg=0x98; /* 16bit 15sps PGA1 */
    i2c_write_blocking(I2C_PORT,ADC_ADDR,&cfg,1,false);
    sleep_ms(200);
}
static void init_lcd(void)
{
    uint8_t cmds[]={0x38,0x39,0x14,0x70,0x56,0x6C,0x38,0x0C,0x01};
    uint8_t d[2]={0x00,0};
    for(uint8_t i=0;i<sizeof cmds;++i){
        d[1]=cmds[i];
        i2c_write_blocking(I2C_PORT,LCD_ADDR,d,2,false);
        sleep_ms((i==5)?200:((i==8)?2:0));
    }
}
static void init_buzzer(void)
{
    gpio_set_function(BUZZER_GPIO,GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(BUZZER_GPIO);
    pwm_set_clkdiv(slice,125.f); pwm_set_wrap(slice,999);
    pwm_set_chan_level(slice,pwm_gpio_to_channel(BUZZER_GPIO),500);
    pwm_set_enabled(slice,false);
}

// ── モード切替 GPIO ───────────────────────────
static void set_mode_pins(meas_mode_t mode)
{
    if (mode==MODE_RESISTANCE){
        gpio_put(MOS_GP3,1); gpio_put(MOS_GP2,0);
        gpio_put(MOS_GP14,0); gpio_put(MOS_GP12,1);
    } else {
        gpio_put(MOS_GP3,0); gpio_put(MOS_GP2,1);
        gpio_put(MOS_GP14,1); gpio_put(MOS_GP12,0);
        gpio_put(R1_GPIO,0); gpio_put(R2_GPIO,1);
        gpio_put(R3_GPIO,0); gpio_put(R4_GPIO,0);
    }
}
static void blink_led(uint8_t times,uint32_t ms)
{
    for(uint8_t i=0;i<times;++i){
        gpio_put(LED_GPIO,1); sleep_ms(ms);
        gpio_put(LED_GPIO,0); sleep_ms(ms);
    }
}

// ── ADC 読み出し ─────────────────────────────
static int16_t read_adc_raw(void)
{
    uint8_t b[2];
    i2c_read_blocking(I2C_PORT,ADC_ADDR,(char*)b,2,false);
    return (int16_t)((b[0]<<8)|(b[1]&0xFF));
}

// ── LCD ユーティリティ ───────────────────────
static void lcd_set_cursor(uint8_t pos)
{
    uint8_t b[2]={0x00,(uint8_t)(0x80|pos)};
    i2c_write_blocking(I2C_PORT,LCD_ADDR,b,2,false);
}
static void lcd_write_char(char c)
{
    uint8_t b[2]={0x40,(uint8_t)c};
    i2c_write_blocking(I2C_PORT,LCD_ADDR,b,2,false);
}
static void lcd_write_digits(int val,uint8_t d)
{
    char buf[4]={'0','0','0','0'};
    for(int i=0;i<4;++i){ buf[3-i]='0'+(val%10); val/=10; }
    for(uint8_t i=0;i<d;++i) lcd_write_char(buf[4-d+i]);
}

// ── 抵抗モード固定表示 ───────────────────────
static void draw_res_fixed(void)
{
    lcd_set_cursor(4);        lcd_write_char('.');
    lcd_set_cursor(0x40+6);   lcd_write_char('k');
    lcd_set_cursor(0x40+7);   lcd_write_char(30); /* Ω */
}

// ── 表示関数 ─────────────────────────────────
static void display_resistance(uint32_t dut)
{
    lcd_set_cursor(0); lcd_write_digits(dut/1000,4);
    lcd_set_cursor(5); lcd_write_digits(dut%1000,3);
}
/* ───────────────────────────────
 *  電圧表示（2-ADC 版と同一ロジック）
 *    raw   : MCP3425 の 16-bit 生データ
 *    表示例: +12.34V  （8×2 文字 LCD 用）
 * ─────────────────────────────── */
static void display_voltage(int16_t raw)
{
    /* 1) mV に変換（分圧・補正込み）*/
    float val_f = raw * (ATT_ADC1 * RES) * G_CALIB;  // [mV] 浮動小数

    /* 2) 小数点以下を切り捨て（= int16_t キャスト）*/
    int16_t val = (int16_t) val_f;                   // [mV] 整数

    /* 3) LCD 先頭に符号を出力 */
    lcd_set_cursor(0);
    if (raw < 0) {
        lcd_write_char('-');
        val = -val;          /* 表示用に正数へ */
    } else {
        lcd_write_char('+');
    }

    /* 4) 整数部・小数部 2 桁ずつに分解 */
    int int_part  = val / 1000;          // V の整数部 (00-99)
    int frac_part = (val % 1000) / 10;   // 小数第 1-2 位 (00-99)

    /* 5) "+xx.xxV " を 8 文字で出力 */
    lcd_write_digits(int_part, 2);       // 1-2 桁目
    lcd_write_char('.');
    lcd_write_digits(frac_part, 2);      // 4-5 桁目
    lcd_write_char('V');
    lcd_write_char(' ');                 // 余白


    /* 6) UART へデバッグ出力（常時有効） */
    printf("raw=%6d  %.4f mV  (%.2f V)\r\n",
           raw,            /* ADC 生値 */
           val_f,          /* mV (float) */
           val_f / 1000.0f /* V  */);

}


// ── 抵抗レンジ制御 ───────────────────────────
static void range_sw(uint8_t ch)
{
    if      (ch==0){ gpio_put(R1_GPIO,1); gpio_put(R2_GPIO,0);
                     gpio_put(R3_GPIO,0); gpio_put(R4_GPIO,0); }
    else if (ch==1){ gpio_put(R1_GPIO,0); gpio_put(R2_GPIO,1);
                     gpio_put(R3_GPIO,0); gpio_put(R4_GPIO,0); }
    else if (ch==2){ gpio_put(R1_GPIO,0); gpio_put(R2_GPIO,0);
                     gpio_put(R3_GPIO,1); gpio_put(R4_GPIO,0); }
    else if (ch==3){ gpio_put(R1_GPIO,0); gpio_put(R2_GPIO,0);
                     gpio_put(R3_GPIO,0); gpio_put(R4_GPIO,1); }
}
