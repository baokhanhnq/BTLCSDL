#ifndef HCSR04_H_
#define HCSR04_H_

#include <stdint.h>

/*
 * Cau hinh PWM trigger, GPIO ECHO va ngat EXTI1 cho HC-SR04.
 */
void HCSR04_Init(void);

/*
 * API doc chu ky cu duoc giu lai cho code cu.
 */
void HCSR04_Process(void);

/*
 * API ngat ECHO cu duoc giu lai cho code cu.
 */
void HCSR04_ECHO_IRQHandler(void);

/*
 * Callback ngat ECHO uu tien, duoc goi tu EXTI1_IRQHandler().
 */

#endif /* HCSR04_H_ */
