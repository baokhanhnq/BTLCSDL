#ifndef BSW_ECUAL_SENSORS_HCSR04_H_
#define BSW_ECUAL_SENSORS_HCSR04_H_

#include "stm32f4xx.h"
#include <stdint.h>

void HCSR04_Init(void);

/* Preferred cyclic API: call every 10ms. */
void HCSR04_Process(void);

/* Preferred EXTI callback: call from EXTI1_IRQHandler(). */
void HCSR04_EchoIrqHandler(void);

/* Compatibility aliases for the previous AEB14 branch naming. */
void Hcsr04_GetDistance(void);
void HCSR04_ECHO_IRQHandler(void);

/* Legacy read APIs kept for older callers. */
uint32_t HCSR04_Read(void);
uint32_t HCSR04_ReadCm10(void);

#endif /* BSW_ECUAL_SENSORS_HCSR04_H_ */
