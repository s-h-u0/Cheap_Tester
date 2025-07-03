#include "hw_i2c.h"
#include "config.h"

void hw_i2c_init(void)
{
    i2c_init(I2C_PORT, 100 * 1000);   /* 100 kHz */
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
    sleep_ms(200);
}
