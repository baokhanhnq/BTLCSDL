#ifndef HCSR04_H_
#define HCSR04_H_

/*
 * Cau hinh PWM trigger, GPIO ECHO va ngat EXTI1 cho HC-SR04.
 */
void HCSR04_Init(void);

/*
 * API cho tac vu cam bien chu ky. Goi moi 20 ms.
 */
void HCSR04_Process(void);

/*
 * Callback ngat ECHO uu tien, duoc goi tu EXTI1_IRQHandler().
 */
void HCSR04_EchoIrqHandler(void);

#endif /* HCSR04_H_ */
