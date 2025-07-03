#ifndef HW_ADC_H_
#define HW_ADC_H_
#include <stdint.h>
void    hw_adc_init(void);
int16_t hw_adc_read_raw(void);
#endif