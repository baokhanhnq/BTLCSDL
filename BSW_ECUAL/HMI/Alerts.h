#ifndef BSW_ECUAL_HMI_ALERTS_H_
#define BSW_ECUAL_HMI_ALERTS_H_

#include "stm32f4xx.h"
#include <stdbool.h>

void Alerts_Init(void);
void Alerts_SetSolid(bool green, bool yellow, bool red, bool buzzer);
void Alerts_UpdateBlink(bool green,
                        bool yellow,
                        bool red,
                        bool buzzer,
                        uint32_t interval_ms);
void Alert_Execute(void);

#endif /* BSW_ECUAL_HMI_ALERTS_H_ */
