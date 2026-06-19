#include "delay.h"

static uint32_t get_tim2_clock_hz(void) {
    uint32_t hclk = HAL_RCC_GetHCLKFreq();
    uint32_t ppre1 = (RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos;
    uint32_t apb1_div;

    if (ppre1 < 4U) {
        apb1_div = 1U;
    } else {
        apb1_div = 1U << (ppre1 - 3U);
    }

    uint32_t pclk1 = hclk / apb1_div;
    return (apb1_div == 1U) ? pclk1 : (pclk1 * 2U);
}

void delay_init(void) {
    /* Cap nhat SystemCoreClock de dam bao gia tri dung. */
    SystemCoreClockUpdate();
    
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = (get_tim2_clock_hz() / 1000000U) - 1U;
    TIM2->EGR |= TIM_EGR_UG; /* Ep update de nap prescaler ngay. */
    TIM2->ARR = 0xFFFFFFFF;
    TIM2->CNT = 0;
    TIM2->CR1 |= TIM_CR1_CEN;
}

void delay_us(uint32_t us) {
    TIM2->CNT = 0;
    while (TIM2->CNT < us);
}

void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        delay_us(1000);
    }
}
