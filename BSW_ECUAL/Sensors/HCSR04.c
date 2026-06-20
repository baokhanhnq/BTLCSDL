#include "HCSR04.h"
#include "Rte.h"
#include "stm32f4xx.h"

/* PA0: ngo ra trigger TIM2_CH1. */
#define TRIG_PIN            0U

/* PA1: ngo vao ECHO qua ngat EXTI1. */
#define ECHO_PIN            1U

#define ECHO_TIMEOUT_US     20000U
#define TIM2_PERIOD_US      20000U

static volatile uint32_t s_echoRiseTime_us = 0U;
static volatile uint32_t s_echoFallTime_us = 0U;
static volatile uint8_t s_measuring = 0U;
static volatile uint8_t s_dataReady = 0U;

/* Khoang cach hop le cuoi cung theo don vi 0.1 cm. */
static uint32_t s_lastDistance_cm10 = 0U;

static void TIM2_PWM_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->PSC = 83U;
    TIM2->ARR = TIM2_PERIOD_US - 1U;
    TIM2->CCR1 = 10U;

    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM2->CCMR1 |= (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;

    TIM2->CCER |= TIM_CCER_CC1E;
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;
}

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

static void EXTI1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI1;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PA;

    EXTI->RTSR |= (1U << ECHO_PIN);
    EXTI->FTSR &= ~(1U << ECHO_PIN);
    EXTI->IMR |= (1U << ECHO_PIN);
    EXTI->PR = (1U << ECHO_PIN);

    NVIC_SetPriority(EXTI1_IRQn, 1U);
    NVIC_EnableIRQ(EXTI1_IRQn);
}

void HCSR04_Init(void)
{
    /* Khoi tao day duong trigger, echo va timer truoc khi bat dau do. */
    GPIO_HCSR04_Init();
    TIM2_PWM_Init();
    EXTI1_Init();

    /* Xoa trang thai do cu de chu ky dau tien bat dau tu moc sach. */
    s_lastDistance_cm10 = 0U;
    s_echoRiseTime_us = 0U;
    s_echoFallTime_us = 0U;
    s_measuring = 0U;
    s_dataReady = 0U;
}

void HCSR04_Process(void)
{
    uint32_t current_distance_cm10;
    uint32_t rise;
    uint32_t fall;
    uint32_t width_us;

    current_distance_cm10 = s_lastDistance_cm10;
    rise = 0U;
    fall = 0U;
    width_us = 0U;

    /* Neu chu ky truoc van dang do thi coi la timeout va giu gia tri cu. */
    if (s_measuring != 0U)
    {
        s_measuring = 0U;
        s_dataReady = 0U;

        /* Dua EXTI ve trang thai bat suon len cho lan trigger tiep theo. */
        EXTI->RTSR |= (1U << ECHO_PIN);
        EXTI->FTSR &= ~(1U << ECHO_PIN);
    }
    /* Neu ISR da bat duoc ca suon len va suon xuong thi tinh do rong xung. */
    else if (s_dataReady != 0U)
    {
        s_dataReady = 0U;

        rise = s_echoRiseTime_us;
        fall = s_echoFallTime_us;

        /* TIM2 chay vong 20 ms nen can xu ly ca truong hop CNT bi tran. */
        if (fall >= rise)
        {
            width_us = fall - rise;
        }
        else
        {
            width_us = (TIM2_PERIOD_US - rise) + fall;
        }

        /* Chi cap nhat khi xung echo hop le, neu khong tiep tuc giu gia tri cu. */
        if ((width_us > 0U) && (width_us < ECHO_TIMEOUT_US))
        {
            current_distance_cm10 = ((width_us * 10U) + 29U) / 58U;
            s_lastDistance_cm10 = current_distance_cm10;
        }
    }
    else
    {
        /* Giu khoang cach hop le gan nhat. */
    }

    /* Ghi ca don vi 0.1 cm de log va cm lam tron de AEB xu ly. */
    Rte_Write_RawDistanceCm10((uint16_t)current_distance_cm10);
    Rte_Write_RawDistance((uint16_t)((current_distance_cm10 + 5U) / 10U));

    /* Mo chu ky do moi, cho ISR bat suon echo cua lan trigger tiep theo. */
    EXTI->PR = (1U << ECHO_PIN);
    s_measuring = 1U;
}

void HCSR04_EchoIrqHandler(void)
{
    uint32_t timestamp;
    uint32_t echo_mask;

    timestamp = 0U;
    echo_mask = (1U << ECHO_PIN);

    if ((EXTI->PR & echo_mask) != 0U)
    {
        /* Xoa co ngat truoc, sau do chup moc thoi gian bang TIM2->CNT. */
        EXTI->PR = echo_mask;
        timestamp = TIM2->CNT;

        /* Suon len: luu moc bat dau xung echo va doi sang bat suon xuong. */
        if ((EXTI->RTSR & echo_mask) != 0U)
        {
            s_echoRiseTime_us = timestamp;

            EXTI->RTSR &= ~echo_mask;
            EXTI->FTSR |= echo_mask;
        }
        /* Suon xuong: luu moc ket thuc xung echo va bao process co du lieu. */
        else if ((EXTI->FTSR & echo_mask) != 0U)
        {
            if (s_measuring != 0U)
            {
                s_echoFallTime_us = timestamp;
                s_dataReady = 1U;
                s_measuring = 0U;
            }

            /* Quay lai bat suon len de san sang cho chu ky trigger sau. */
            EXTI->FTSR &= ~echo_mask;
            EXTI->RTSR |= echo_mask;
        }
        else
        {
            /* Khong co canh nao duoc cau hinh. */
        }
    }
}
