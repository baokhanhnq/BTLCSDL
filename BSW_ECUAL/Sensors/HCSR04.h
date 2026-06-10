#ifndef BSW_ECUAL_SENSORS_HCSR04_H_
#define BSW_ECUAL_SENSORS_HCSR04_H_

#include "stm32f4xx.h"
#include <stdint.h>

void HCSR04_Init(void);
uint32_t HCSR04_Read(void);
uint32_t HCSR04_ReadCm10(void);

#endif /* BSW_ECUAL_SENSORS_HCSR04_H_ */
