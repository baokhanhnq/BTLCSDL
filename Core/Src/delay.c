/**
  ******************************************************************************
  * @file           : delay.c
  * @brief          : DWT Microsecond Delay Common Utility Implementation.
  ******************************************************************************
  */

#include "delay.h"

/**
 * @brief Initializes the DWT (Data Watchpoint and Trace) hardware block.
 */
void Delay_DWT_Init(void) {
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/**
 * @brief Block execution for a cycle-accurate microsecond duration.
 * @param us Duration in microseconds
 */
void delay_us(uint16_t us) {
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = (uint32_t)us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - startTick) < delayTicks) {
        // Wait cycle
    }
}
