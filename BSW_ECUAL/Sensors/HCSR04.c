#include "HCSR04.h"
#include "Rte.h"
#include "stm32f4xx.h"

/* PA0: ngo ra trigger TIM2_CH1. */
#define TRIG_PIN            0U

/* PA1: ngo vao ECHO qua ngat EXTI1. */
#define ECHO_PIN            1U

/* Echo phai ket thuc trong mot chu ky trigger phan cung 20 ms. */
#define ECHO_TIMEOUT_US     20000U
#define TIM2_PERIOD_US      20000U

static volatile uint32_t s_echoRiseTime_us = 0U;
static volatile uint32_t s_triggerTime_us = 0U;
static volatile uint8_t s_measuring = 0U;

/* Khoang cach hop le cuoi cung theo don vi 0.1 cm. Vi du: 123 la 12.3 cm. */
static uint32_t s_lastDistance_cm10 = 0U;

/*
 * Luu ca hai dinh dang khoang cach vao RTE.
 * RawDistanceCm10 giu do chinh xac 0.1 cm de log.
 * RawDistance duoc lam tron ve cm cho logic AEB.
 */
static void HCSR04_WriteDistance(uint32_t distance_cm10)
{
    s_lastDistance_cm10 = distance_cm10;
    Rte_Write_RawDistanceCm10((uint16_t)distance_cm10);
    Rte_Write_RawDistance((uint16_t)((distance_cm10 + 5U) / 10U));
}

/*
 * Cau hinh TIM2 CH1 tao xung trigger 10 us khong block CPU
 * moi 20 ms tren PA0.
 */
static void TIM2_PWM_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* 84 MHz / 84 = 1 MHz, moi tick cua TIM2 la 1 us. */
    TIM2->PSC = 83U;
    TIM2->ARR = TIM2_PERIOD_US - 1U;
    TIM2->CCR1 = 10U;

    /* PWM mode 1: PA0 muc cao khi CNT < CCR1. */
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM2->CCMR1 |= (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;

    TIM2->CCER |= TIM_CCER_CC1E;
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;
}

/*
 * Cau hinh PA0 la chuc nang thay the TIM2_CH1 va PA1 la ngo vao ECHO.
 */
static void GPIO_HCSR04_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    GPIOA->MODER &= ~(3U << (TRIG_PIN * 2U));
    GPIOA->MODER |= (2U << (TRIG_PIN * 2U));

    GPIOA->AFR[0] &= ~(0xFU << (TRIG_PIN * 4U));
    GPIOA->AFR[0] |= (1U << (TRIG_PIN * 4U));

    GPIOA->MODER &= ~(3U << (ECHO_PIN * 2U));
    GPIOA->PUPDR &= ~(3U << (ECHO_PIN * 2U));
}

/*
 * Cau hinh EXTI1 bat ca canh len va canh xuong tren PA1.
 */
static void EXTI1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI1;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PA;

    EXTI->RTSR |= (1U << ECHO_PIN);
    EXTI->FTSR |= (1U << ECHO_PIN);
    EXTI->IMR |= (1U << ECHO_PIN);
    EXTI->PR = (1U << ECHO_PIN);

    NVIC_SetPriority(EXTI1_IRQn, 1U);
    NVIC_EnableIRQ(EXTI1_IRQn);
}

/*
 * Khoi tao PWM trigger, GPIO ECHO va ngat ECHO cho HC-SR04.
 */
void HCSR04_Init(void)
{
    GPIO_HCSR04_Init();
    TIM2_PWM_Init();
    EXTI1_Init();

    s_lastDistance_cm10 = 0U;
    s_echoRiseTime_us = 0U;
    s_triggerTime_us = 0U;
    s_measuring = 0U;
}

/*
 * Chay moi 20 ms.
 * Neu xung ECHO truoc chua ket thuc, giu khoang cach hop le cuoi.
 * Sau do chuan bi trang thai cho chu ky trigger phan cung moi.
 */
void Hcsr04_CheckTimeout_20ms(void)
{
    if (s_measuring != 0U)
    {
        s_measuring = 0U;
        HCSR04_WriteDistance(s_lastDistance_cm10);
    }

    EXTI->PR = (1U << ECHO_PIN);
    s_triggerTime_us = TIM2->CNT;
    s_measuring = 1U;
}

/*
 * API cu duoc giu lai cho code cu.
 */
void Hcsr04_GetDistance(void)
{
    Hcsr04_CheckTimeout_20ms();
}

/*
 * API chu ky duoc goi tu tac vu cam bien.
 */
void HCSR04_Process(void)
{
    Hcsr04_CheckTimeout_20ms();
}

/*
 * Xu ly cac canh ECHO.
 * Canh len ghi thoi diem bat dau; canh xuong doi do rong xung sang khoang cach.
 */
void HCSR04_ECHO_IRQHandler(void)
{
    uint32_t timestamp;
    uint32_t width_us;

    if ((EXTI->PR & (1U << ECHO_PIN)) == 0U)
    {
        return;
    }

    EXTI->PR = (1U << ECHO_PIN);
    timestamp = TIM2->CNT;

    if ((GPIOA->IDR & (1U << ECHO_PIN)) != 0U)
    {
        s_echoRiseTime_us = timestamp;
        return;
    }

    if (s_measuring == 0U)
    {
        return;
    }

    if (timestamp >= s_echoRiseTime_us)
    {
        width_us = timestamp - s_echoRiseTime_us;
    }
    else
    {
        width_us = (TIM2_PERIOD_US - s_echoRiseTime_us) + timestamp;
    }

    if ((width_us > 0U) && (width_us < ECHO_TIMEOUT_US))
    {
        HCSR04_WriteDistance(((width_us * 10U) + 29U) / 58U);
    }
    else
    {
        HCSR04_WriteDistance(s_lastDistance_cm10);
    }

    s_measuring = 0U;
}

/*
 * Ten callback ngat uu tien duoc stm32f4xx_it.c su dung.
 */
void HCSR04_EchoIrqHandler(void)
{
    HCSR04_ECHO_IRQHandler();
}
