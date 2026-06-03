#ifndef UART_H
#define UART_H

#include "stm32f4xx.h"

void UART2_Init(void);
void UART_SendChar(char c);
void UART_SendString(char *str);
void UART_SendNumber(uint32_t num);

#endif
