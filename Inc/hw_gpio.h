#ifndef HW_GPIO_H_
#define HW_GPIO_H_
#include <stdbool.h>
#include <stdint.h>
void hw_gpio_init(void);
void hw_led_blink(uint8_t t, uint32_t ms);
void hw_buzzer_set(bool on);
#endif
