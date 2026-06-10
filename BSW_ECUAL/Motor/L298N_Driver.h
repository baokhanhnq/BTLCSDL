#ifndef BSW_ECUAL_MOTOR_L298N_DRIVER_H_
#define BSW_ECUAL_MOTOR_L298N_DRIVER_H_

#include "stm32f4xx.h"
#include <stdint.h>

void L298N_Init(void);
void L298N_SetSpeed(uint16_t duty);
void L298N_ApplyBrake(void);

#endif /* BSW_ECUAL_MOTOR_L298N_DRIVER_H_ */
