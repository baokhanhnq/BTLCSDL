#ifndef INC_DELAY_H_
#define INC_DELAY_H_

#include "stm32f4xx.h"
#include <stdint.h>

void delay_init(void);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

#endif /* INC_DELAY_H_ */
