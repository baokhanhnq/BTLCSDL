#include "adc.h"

void ADC1_Init(void)
{
    /* Enable GPIOA and ADC1 clocks */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    volatile uint32_t dummy = RCC->APB2ENR;
    (void)dummy;

    /* Configure PA4 as analog input for ADC1_IN4 throttle */
    GPIOA->MODER &= ~GPIO_MODER_MODE4_Msk;
    GPIOA->MODER |= GPIO_MODER_MODE4_Msk;
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD4_Msk;

    /* One conversion: channel 4 */
    ADC1->SQR1 = 0;
    ADC1->SQR3 = 4U;

    /* 480-cycle sample time for channel 4 */
    ADC1->SMPR2 &= ~ADC_SMPR2_SMP4_Msk;
    ADC1->SMPR2 |= (7U << ADC_SMPR2_SMP4_Pos);

    /* Enable ADC */
    ADC1->CR2 |= ADC_CR2_ADON;
}

uint16_t ADC1_Read(void)
{
    ADC1->CR2 |= ADC_CR2_SWSTART;

    while (!(ADC1->SR & ADC_SR_EOC));

    return (uint16_t)ADC1->DR;
}
