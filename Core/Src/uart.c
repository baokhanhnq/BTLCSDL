/**
  ******************************************************************************
  * @file           : uart.c
  * @brief          : UART Serial Debug Logging Utility Implementation.
  ******************************************************************************
  */

#include "uart.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
static UART_HandleTypeDef *g_huart = NULL;

/**
 * @brief Binds the utility to the configured UART handle.
 * @param huart UART Peripheral handle (e.g. &huart2)
 */
void UART_Init(UART_HandleTypeDef *huart) {
    g_huart = huart;
}

/**
 * @brief Transmits a single character over serial UART.
 * @param c Character to transmit
 */
void UART_SendChar(char c) {
    if (g_huart == NULL) return;
    HAL_UART_Transmit(g_huart, (uint8_t *)&c, 1, 10);
}

/**
 * @brief Transmits a null-terminated string over serial UART.
 * @param str Pointer to string
 */
void UART_SendString(const char *str) {
    if (str == NULL || g_huart == NULL) return;
    HAL_UART_Transmit(g_huart, (uint8_t *)str, strlen(str), 100);
}

/**
 * @brief Formats and transmits a 32-bit unsigned integer over serial UART.
 * @param num Number to transmit
 */
void UART_SendNumber(uint32_t num) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)num);
    UART_SendString(buf);
}

/**
 * @brief Custom printf formatting helper to print structured text to terminal.
 * @param format Printf format specifier string
 * @param ... Variable argument list
 */
void UART_Printf(const char *format, ...) {
    if (format == NULL || g_huart == NULL) return;
    
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    
    UART_SendString(buf);
}
