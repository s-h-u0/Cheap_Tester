#include "hw_adc.h"
#include "config.h"
#include "hardware/i2c.h"

void hw_adc_init(void)
{
    uint8_t cfg = 0x98;  /* 16‑bit, 15 sps, PGA×1 */
    i2c_write_blocking(I2C_PORT, ADC_ADDR, &cfg, 1, false);
    sleep_ms(200);
}

int16_t hw_adc_read_raw(void)
{
    uint8_t b[2];
    i2c_read_blocking(I2C_PORT, ADC_ADDR, (char *)b, 2, false);
    return (int16_t)((b[0] << 8) | (b[1] & 0xFF));
}