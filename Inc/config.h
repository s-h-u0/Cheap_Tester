#ifndef CONFIG_H_
#define CONFIG_H_
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/* --- GPIO ------------------------------------------------------------- */
#define Q1_GPIO      11
#define Q2_GPIO      12
#define Q3_GPIO      13
#define Q4_GPIO      14
#define Q5_GPIO      16
#define Q6_GPIO      20
#define Q7_GPIO      15
#define Q8_GPIO      22

/* Measurement range aliases */
#define R1_GPIO      Q1_GPIO
#define R2_GPIO      Q2_GPIO
#define R3_GPIO      Q3_GPIO
#define R4_GPIO      Q4_GPIO

#define BUZZER_GPIO   5
#define LED_GPIO     25
#define SW_RES_GPIO  17
#define SW_VOL_GPIO  18

/* --- I²C -------------------------------------------------------------- */
#define I2C_PORT  i2c1
#define PIN_SDA   26
#define PIN_SCL   27
#define LCD_ADDR  0x3E    /* AQM0802A */
#define ADC_ADDR  0x68    /* MCP3425  */
#endif /* CONFIG_H_ */