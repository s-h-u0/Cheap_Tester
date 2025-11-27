#include "hw_gpio.h"
#include "config.h"
#include "hardware/pwm.h"

/* ---- 内部ヘルパ ------------------------------------------------------ */
static inline uint _buzzer_slice(void)
{
    return pwm_gpio_to_slice_num(BUZZER_GPIO);
}

void hw_gpio_init(void)
{
    /* --- 出力ポート初期化 ------------------------------------------- */
    const uint outs[] = {
        R1_GPIO, R2_GPIO, R3_GPIO, R4_GPIO,
        MOS_GP3, MOS_GP2, MOS_GP14, SHDN_GP12
    };
    for (uint i = 0; i < sizeof outs / sizeof outs[0]; ++i) {
        gpio_init(outs[i]);
        gpio_set_dir(outs[i], GPIO_OUT);
        gpio_put(outs[i], 0);
    }

    /* --- 入力ポート初期化（プルアップ） ----------------------------- */
    const uint ins[] = { SW_RES_GPIO, SW_VOL_GPIO };
    for (uint i = 0; i < sizeof ins / sizeof ins[0]; ++i) {
        gpio_init(ins[i]);
        gpio_set_dir(ins[i], GPIO_IN);
        gpio_pull_up(ins[i]);
    }

    /* --- LED -------------------------------------------------------- */
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);

    /* --- ブザー (PWM) ---------------------------------------------- */
    gpio_set_function(BUZZER_GPIO, GPIO_FUNC_PWM);
    uint slice = _buzzer_slice();
    pwm_set_clkdiv(slice, 125.f);  /* ≒ 1 kHz */
    pwm_set_wrap(slice, 999);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(BUZZER_GPIO), 500);
    pwm_set_enabled(slice, false);
}

void hw_led_blink(uint8_t times, uint32_t ms)
{
    while (times--) {
        gpio_put(LED_GPIO, 1);
        sleep_ms(ms);
        gpio_put(LED_GPIO, 0);
        sleep_ms(ms);
    }
}

void hw_buzzer_set(bool on)
{
    pwm_set_enabled(_buzzer_slice(), on);
}