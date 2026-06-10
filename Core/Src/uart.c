#include "uart.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static UART_HandleTypeDef *g_huart = NULL;

void UART2_Init(UART_HandleTypeDef *huart) {
    g_huart = huart;
}

void UART_SendChar(char c) {
    if (g_huart != NULL) {
        HAL_UART_Transmit(g_huart, (uint8_t *)&c, 1, HAL_MAX_DELAY);
    }
}

void UART_SendString(const char* str) {
    if (str != NULL && g_huart != NULL) {
        HAL_UART_Transmit(g_huart, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
    }
}

void UART_SendNumber(uint32_t num) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)num);
    UART_SendString(buf);
}

void UART_Printf(const char *format, ...) {
    if (format == NULL || g_huart == NULL) return;

    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    UART_SendString(buf);
}
