#include "app.h"
#include "uart.h"
#include "delay.h"
#include "ultrasonic.h"

void App_Init(void)
{
    UART2_Init();
    TIM5_Delay_Init();
    Ultrasonic_Init();

    UART_SendString("HC-SR04 Ready\r\n");
}

void App_Run(void)
{
    uint32_t distance_x100;

    distance_x100 = Ultrasonic_GetDistance_x100();

    UART_SendString("Distance = ");

    UART_SendNumber(distance_x100 / 100);

    UART_SendChar('.');

    if((distance_x100 % 100) < 10)
    {
        UART_SendChar('0');
    }

    UART_SendNumber(distance_x100 % 100);

    UART_SendString(" cm\r\n");

    delay_us(30000);
}
