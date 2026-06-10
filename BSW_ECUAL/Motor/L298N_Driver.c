#include "L298N_Driver.h"

void L298N_Init(void) {
    /* 1. Enable Clocks for GPIOA, GPIOB and TIM1 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    /* 2. Configure PA8 to Alternate Function Mode (for TIM1_CH1 PWM output) */
    GPIOA->MODER &= ~(3U << (8 * 2));
    GPIOA->MODER |= (2U << (8 * 2));
    GPIOA->AFR[1] &= ~(0xFU << (0 * 4));
    GPIOA->AFR[1] |= (1U << (0 * 4)); /* AF1 = TIM1 */

    /* 3. Configure PB0 and PB1 as General Purpose Output Pins (for L298N IN1, IN2) */
    GPIOB->MODER &= ~((3U << (0 * 2)) | (3U << (1 * 2)));
    GPIOB->MODER |= (1U << (0 * 2)) | (1U << (1 * 2));

    /* 4. Configure TIM1 */
    TIM1->PSC = 84 - 1;     /* 84MHz TIM1 clock prescaled to 1MHz */
    TIM1->ARR = 1000 - 1;   /* PWM period = 1ms (1kHz frequency) */
    TIM1->CCR1 = 0;     /* Default Duty Cycle = 0% */

    /* PWM mode 1 (active while CNT < CCR) */
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM1->CCMR1 |= (6U << TIM_CCMR1_OC1M_Pos);
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE; /* Enable preload register */

    TIM1->CCER |= TIM_CCER_CC1E; /* Enable TIM1 CH1 Output */
    TIM1->BDTR |= TIM_BDTR_MOE;  /* Main Output Enable (Required for Advanced TIM1) */
    TIM1->CR1 |= TIM_CR1_CEN;    /* Enable TIM1 Counter */

    /* Set default direction (IN1 = High, IN2 = Low) */
    GPIOB->BSRR = (1U << 0);
    GPIOB->BSRR = (1U << (1 + 16));
}

void L298N_SetSpeed(uint16_t duty) {
    if (duty > 100) duty = 100;
    
    /* Ensure direction pins are set for forward movement */
    GPIOB->BSRR = (1U << 0);
    GPIOB->BSRR = (1U << (1 + 16));

    /* Set TIM1 CH1 Capture Compare Register value */
    TIM1->CCR1 = ((TIM1->ARR + 1U) * duty) / 100;
}

void L298N_ApplyBrake(void) {
    /* Set speed duty cycle to 0% */
    TIM1->CCR1 = 0;
    
    /* Pull both control pins Low to brake */
    GPIOB->BSRR = (1U << (0 + 16)) | (1U << (1 + 16));
}
