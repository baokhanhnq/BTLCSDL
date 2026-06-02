/**
  ******************************************************************************
  * @file           : uart.h
  * @brief          : UART Serial Debug Logging Utility Header.
  ******************************************************************************
  */

#ifndef INC_UART_H_
#define INC_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>

/* Exported Functions --------------------------------------------------------*/

/**
 * @brief Binds the utility to the configured UART handle.
 * @param huart UART Peripheral handle (e.g. &huart2)
 */
void UART_Init(UART_HandleTypeDef *huart);

/**
 * @brief Transmits a single character over serial UART.
 * @param c Character to transmit
 */
void UART_SendChar(char c);

/**
 * @brief Transmits a null-terminated string over serial UART.
 * @param str Pointer to string
 */
void UART_SendString(const char *str);

/**
 * @brief Formats and transmits a 32-bit unsigned integer over serial UART.
 * @param num Number to transmit
 */
void UART_SendNumber(uint32_t num);

/**
 * @brief Custom printf formatting helper to print structured text to terminal.
 * @param format Printf format specifier string
 * @param ... Variable argument list
 */
void UART_Printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* INC_UART_H_ */
