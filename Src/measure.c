#include "measure.h"
#include "config.h"
#include "hw_adc.h"
#include "pico/stdlib.h"

#define RESOLUTION_FACTOR (2.048f / 32768.f * 1000.f) /* mV per LSB */

/* 実測ゲイン補正係数 (ch0‑ch3) */
static const float gain_tbl[4] = {
    0.986f, 0.975f, 1.000f, 0.668f
};

/* --------------------------------------------------------------------
 * range_gpio_set()
 *   ch = 0‑3 に応じて R1‑R4 を切替
 * ------------------------------------------------------------------ */
static inline void range_gpio_set(uint8_t ch)
{
    gpio_put(R1_GPIO, ch == 0);
    gpio_put(R2_GPIO, ch == 1);
    gpio_put(R3_GPIO, ch == 2);
    gpio_put(R4_GPIO, ch == 3);
}

/* ============================  公開 API  ============================ */
void measure_init(void)
{
    hw_adc_init();
}

uint32_t measure_resistance(uint8_t *used_range)
{
    static uint8_t ch = 0;
    float   mv;

    /* ---- 自動レンジ探索 ------------------------------------------ */
    while (true) {
        range_gpio_set(ch);
        sleep_ms(100);                          /* セトルタイム */

        mv = hw_adc_read_raw() * RESOLUTION_FACTOR;

        if ((mv > 500 && mv < 1500) ||          /* 適正レンジ   */
            (mv <= 500 && ch == 3)  ||          /* 最低レンジ   */
            (mv >= 1500 && ch == 0))            /* 最高レンジ   */
        {
            break;                              /* レンジ確定   */
        }
        ch += (mv <= 500) ? 1 : (uint8_t)-1;    /* レンジ ↑ / ↓ */
    }

    if (used_range) *used_range = ch;

    /* ---- 抵抗値計算 (Ω) ----------------------------------------- */
    float dut;
    if      (ch == 0) dut =      1000.f * (2048.f - mv) / mv;
    else if (ch == 1) dut =     10000.f * (2048.f - mv) / mv;
    else if (ch == 2) dut =    100000.f * (2048.f - mv) / mv;
    else               dut =   1000000.f * (2048.f - mv) / mv;

    dut *= gain_tbl[ch];
    return (uint32_t)(dut + 0.5f);              /* 四捨五入して整数 */
}

int16_t measure_voltage(void)
{
    return hw_adc_read_raw();
}
