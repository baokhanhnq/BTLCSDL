#include "delay.h"

void TIM5_Delay_Init(void)
{
    RCC->APB1ENR |= (1<<3);

    TIM5->PSC = 16 - 1;
    TIM5->ARR = 0xFFFFFFFF;

    TIM5->CR1 |= TIM_CR1_CEN;
}

void delay_us(uint32_t us)
{
    TIM5->CNT = 0;

    while(TIM5->CNT < us);
}
