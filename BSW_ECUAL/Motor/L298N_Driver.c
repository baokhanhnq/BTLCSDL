#include "L298N_Driver.h"

/*
 * Khoi tao chan dieu khien huong va ngo ra PWM TIM1 cho L298N.
 */
void L298N_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    /* PA8: ngo ra PWM TIM1_CH1. */
    GPIOA->MODER &= ~(3U << (8U * 2U));
    GPIOA->MODER |= (2U << (8U * 2U));
    GPIOA->AFR[1] &= ~(0xFU << (0U * 4U));
    GPIOA->AFR[1] |= (1U << (0U * 4U));

    /* PB0/PB1: chan dieu khien huong IN1/IN2 cua L298N. */
    GPIOB->MODER &= ~((3U << (0U * 2U)) | (3U << (1U * 2U)));
    GPIOB->MODER |= ((1U << (0U * 2U)) | (1U << (1U * 2U)));

    /* Clock timer 1 MHz, chu ky PWM 1 kHz. */
    TIM1->PSC = 84U - 1U;
    TIM1->ARR = 1000U - 1U;
    TIM1->CCR1 = 0U;

    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM1->CCMR1 |= (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
    TIM1->CCER |= TIM_CCER_CC1E;
    TIM1->BDTR |= TIM_BDTR_MOE;
    TIM1->CR1 |= TIM_CR1_CEN;

    /* Chieu tien: IN1 muc cao, IN2 muc thap. */
    GPIOB->BSRR = (1U << 0U);
    GPIOB->BSRR = (1U << (1U + 16U));
}

/*
 * Dat toc do motor chieu tien tu 0 den 100 phan tram.
 */
void L298N_SetSpeed(uint16_t duty)
{
    if (duty > 100U)
    {
        duty = 100U;
    }

    GPIOB->BSRR = (1U << 0U);
    GPIOB->BSRR = (1U << (1U + 16U));

    TIM1->CCR1 = ((TIM1->ARR + 1U) * duty) / 100U;
}

/*
 * Dung PWM va keo ca hai chan dieu khien xuong muc thap de phanh.
 */
void L298N_ApplyBrake(void)
{
    TIM1->CCR1 = 0U;
    GPIOB->BSRR = (1U << (0U + 16U)) | (1U << (1U + 16U));
}
