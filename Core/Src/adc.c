#include "adc.h"

void ADC1_Init(void)
{
    /* Bat xung nhip cho GPIOA va ADC1. */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    volatile uint32_t dummy = RCC->APB2ENR;
    (void)dummy;

    /* Cau hinh PA4 lam ngo vao analog ADC1_IN4 cho ga. */
    GPIOA->MODER &= ~GPIO_MODER_MODE4_Msk;
    GPIOA->MODER |= GPIO_MODER_MODE4_Msk;
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD4_Msk;

    /* Mot lan chuyen doi tren kenh 4. */
    ADC1->SQR1 = 0;
    ADC1->SQR3 = 4U;

    /* Thoi gian lay mau 480 chu ky cho kenh 4. */
    ADC1->SMPR2 &= ~ADC_SMPR2_SMP4_Msk;
    ADC1->SMPR2 |= (7U << ADC_SMPR2_SMP4_Pos);

    /* Bat ADC. */
    ADC1->CR2 |= ADC_CR2_ADON;
}

uint16_t ADC1_Read(void)
{
    ADC1->CR2 |= ADC_CR2_SWSTART;

    while (!(ADC1->SR & ADC_SR_EOC));

    return (uint16_t)ADC1->DR;
}
