#include "HCSR04.h"
#include "delay.h"

void HCSR04_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* Configure TIM2 to count in microseconds via delay_init(). */
    delay_init();

    /* PA0 = TRIG output, PA1 = ECHO input. */
    GPIOA->MODER &= ~((3U << (0 * 2)) | (3U << (1 * 2)));
    GPIOA->MODER |= (1U << (0 * 2));
}

uint32_t HCSR04_ReadCm10(void) {
    uint32_t time_us = 0;

    /* Trigger a 10us pulse on PA0. */
    GPIOA->BSRR = (1U << 0);
    delay_us(10);
    GPIOA->BSRR = (1U << 16);

    /* Wait for ECHO PA1 to go high. */
    TIM2->CNT = 0;
    while (!(GPIOA->IDR & (1U << 1))) {
        if (TIM2->CNT > 600000U) {
            return 0;
        }
    }

    /* Measure the high pulse width on ECHO PA1. */
    TIM2->CNT = 0;
    while ((GPIOA->IDR & (1U << 1))) {
        if (TIM2->CNT > 600000U) {
            return 0;
        }
    }

    /*
     * HC-SR04 distance in cm is time_us / 58.
     * Return cm x10 so one decimal digit is preserved:
     * example 253 means 25.3 cm.
     */
    time_us = TIM2->CNT;
    return ((time_us * 10U) + 29U) / 58U;
}

uint32_t HCSR04_Read(void) {
    uint32_t distance_cm10 = HCSR04_ReadCm10();
    return (distance_cm10 + 5U) / 10U;
}
