#ifndef CONFIG_H_
#define CONFIG_H_
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/* --- GPIO ------------------------------------------------------------- */
#define R1_GPIO      13
#define R2_GPIO      10
#define R3_GPIO       8
#define R4_GPIO       6
#define MOS_GP3       3
#define MOS_GP2       2
#define MOS_GP14     14
#define MOS_GP12     12
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