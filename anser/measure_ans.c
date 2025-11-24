#include "measure.h"
#include "config.h"
#include "hw_adc.h"
#include "pico/stdlib.h"

/* =========================================================================
 *  定数・テーブル
 * =========================================================================*/
/* ADC 1LSB あたりの電圧 (mV) : 2.048V / 32768 * 1000  ≒ 0.0625 */
#define RESOLUTION_FACTOR  (2.048f / 32768.0f * 1000.0f)

/* 電圧測定用の分圧比 (固定) */
#define ATT_ADC1           (11)

/* 実測によるキャリブレーション係数 */
#define G_CALIB            (0.998f)

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
    if      (ch == 0) dut =      1000.0f * (2048.0f - mv) / mv;
    else if (ch == 1) dut =     10000.0f * (2048.0f - mv) / mv;
    else if (ch == 2) dut =    100000.0f * (2048.0f - mv) / mv;
    else               dut =   1000000.0f * (2048.0f - mv) / mv;

    dut *= gain_tbl[ch];
    return (uint32_t)(dut + 0.5f);              /* 四捨五入して整数 */
}

/* --------------------------------------------------------------------
 * measure_voltage()
 *   キャリブレーション・スケーリングを内部で完了し
 *   実際の mV 値 (±32767 mV) を返す。
 * ------------------------------------------------------------------ */
int16_t measure_voltage(void)
{
    int16_t raw = hw_adc_read_raw();
    float mv_f  = raw * (ATT_ADC1 * RESOLUTION_FACTOR) * G_CALIB;

    /* ±方向に丸め誤差抑制 */
    if (mv_f >= 0.0f)
        mv_f += 0.5f;
    else
        mv_f -= 0.5f;

    return (int16_t)mv_f;
}
