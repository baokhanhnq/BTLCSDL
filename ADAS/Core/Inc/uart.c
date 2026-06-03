#include "uart.h"

void UART2_Init(void)
{
    RCC->AHB1ENR |= (1<<0);
    RCC->APB1ENR |= (1<<17);

    GPIOA->MODER &= ~(15<<4);
    GPIOA->MODER |=  (10<<4);

    GPIOA->AFR[0] &= ~((15<<8) | (15<<12));
    GPIOA->AFR[0] |=  ((7<<8) | (7<<12));

    USART2->BRR = 1667;

    USART2->CR1 |= USART_CR1_TE;
    USART2->CR1 |= USART_CR1_RE;
    USART2->CR1 |= USART_CR1_UE;
}

void UART_SendChar(char c)
{
    while(!(USART2->SR & USART_SR_TXE));

    USART2->DR = c;
}

void UART_SendString(char *str)
{
    while(*str)
    {
        UART_SendChar(*str++);
    }
}

void UART_SendNumber(uint32_t num)
{
    char buf[12];
    int i = 0;

    if(num == 0)
    {
        UART_SendChar('0');
        return;
    }

    while(num)
    {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }

    while(i)
    {
        UART_SendChar(buf[--i]);
    }
}
