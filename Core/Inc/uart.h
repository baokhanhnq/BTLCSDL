#ifndef INC_UART_H_
#define INC_UART_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

void UART2_Init(UART_HandleTypeDef *huart);
void UART_SendChar(char c);
void UART_SendString(const char* str);
void UART_SendNumber(uint32_t num);
void UART_Printf(const char *format, ...);

#endif /* INC_UART_H_ */
