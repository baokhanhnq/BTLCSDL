#ifndef HCSR04_H
#define HCSR04_H

#include "stm32f4xx.h"
#include <stdint.h>

void HCSR04_Init(void);

/* Call every 50ms */
void Hcsr04_GetDistance(void);

/* Call from EXTI1_IRQHandler() */
void HCSR04_ECHO_IRQHandler(void);

#endif