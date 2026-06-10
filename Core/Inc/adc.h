#ifndef INC_ADC_H_
#define INC_ADC_H_

#include "stm32f4xx.h"
#include <stdint.h>

void ADC1_Init(void);
uint16_t ADC1_Read(void);

#endif /* INC_ADC_H_ */
