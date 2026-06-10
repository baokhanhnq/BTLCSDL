#ifndef BSW_ECUAL_HMI_THROTTLE_H_
#define BSW_ECUAL_HMI_THROTTLE_H_

#include <stdint.h>

void Throttle_Init(void);
uint32_t Throttle_ReadAdc(void);
uint16_t Throttle_GetPercent(uint32_t adc_val);

#endif /* BSW_ECUAL_HMI_THROTTLE_H_ */
