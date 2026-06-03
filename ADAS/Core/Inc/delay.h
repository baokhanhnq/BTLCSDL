#ifndef DELAY_H
#define DELAY_H

#include "stm32f4xx.h"

void TIM5_Delay_Init(void);
void delay_us(uint32_t us);

#endif
