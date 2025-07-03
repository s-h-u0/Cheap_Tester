#ifndef MEASURE_H_
#define MEASURE_H_
#include <stdint.h>
void     measure_init(void);
uint32_t measure_resistance(uint8_t *used_range);
int16_t  measure_voltage(void);
#endif
