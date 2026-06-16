#include "HCSR04.h"
#include "Rte.h"

/* PA0 -> TRIG */
#define TRIG_PIN            0U

/* PA1 -> ECHO */
#define ECHO_PIN            1U

/* TIM2: 1us per tick */
#define TIM2_PSC_VALUE      83U

/* Echo timeout */
#define ECHO_TIMEOUT_US     30000U

static volatile uint32_t s_triggerTime_us  = 0U;
static volatile uint32_t s_echoRiseTime_us = 0U;
static volatile uint8_t  s_measuring       = 0U;

/* B? d?m chia th?i gian d? g?i Trigger m?i 50ms trong Task 10ms */
static uint8_t s_trigger_counter = 0U; 

/*------------------------------------------------------------------*/
/* TIM2 */
/*------------------------------------------------------------------*/
static void TIM2_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->PSC = TIM2_PSC_VALUE;
    TIM2->ARR = 0xFFFFFFFFU;

    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 = TIM_CR1_CEN;
}

/*------------------------------------------------------------------*/
/* GPIO */
/*------------------------------------------------------------------*/
static void GPIO_HCSR04_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* PA0 output */
    GPIOA->MODER &= ~(3U << (TRIG_PIN * 2U));
    GPIOA->MODER |=  (1U << (TRIG_PIN * 2U));

    GPIOA->OTYPER &= ~(1U << TRIG_PIN);
    GPIOA->PUPDR  &= ~(3U << (TRIG_PIN * 2U));

    GPIOA->BSRR = (1U << (TRIG_PIN + 16U));

    /* PA1 input */
    GPIOA->MODER &= ~(3U << (ECHO_PIN * 2U));
    GPIOA->PUPDR &= ~(3U << (ECHO_PIN * 2U));
}

/*------------------------------------------------------------------*/
/* EXTI1 */
/*------------------------------------------------------------------*/
static void EXTI1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI1;
    SYSCFG->EXTICR[0] |=  SYSCFG_EXTICR1_EXTI1_PA;

    EXTI->RTSR |= (1U << ECHO_PIN);
    EXTI->FTSR |= (1U << ECHO_PIN);
    EXTI->IMR  |= (1U << ECHO_PIN);

    EXTI->PR = (1U << ECHO_PIN);

    NVIC_SetPriority(EXTI1_IRQn, 1U);
    NVIC_EnableIRQ(EXTI1_IRQn);
}

/*------------------------------------------------------------------*/
/* Init */
/*------------------------------------------------------------------*/
void HCSR04_Init(void)
{
    TIM2_Init();
    GPIO_HCSR04_Init();
    EXTI1_Init();
    s_trigger_counter = 0U;
}

/*------------------------------------------------------------------*/
/* C?p nh?t: Ð?i tên thành Hcsr04_GetDistance và t?i uu cho Task 10ms */
/*------------------------------------------------------------------*/
void Hcsr04_GetDistance(void)
{
    /* Ki?m tra b?o v? timeout n?u dang trong quá trình do */
    if (s_measuring)
    {
        uint32_t elapsed;
        elapsed = TIM2->CNT - s_triggerTime_us;

        if (elapsed >= ECHO_TIMEOUT_US)
        {
            s_measuring = 0U;
            Rte_Write_RawDistanceCm10(0U);
            Rte_Write_RawDistance(0U);
        }
        return; 
    }

    /* Vì Task này g?i m?i 10ms, ta dùng b? d?m d? ch? phát Trigger m?i 50ms (5 * 10ms) */
    s_trigger_counter++;
    if (s_trigger_counter < 5U)
    {
        return; /* Chua d? 50ms, thoát ra d? nhu?ng CPU cho vi?c khác */
    }
    s_trigger_counter = 0U; /* Reset b? d?m chu k? */

    /* B?t d?u chu k? kích ho?t (Trigger) m?i */
    EXTI->PR = (1U << ECHO_PIN);
    s_measuring = 1U;

    /* T?o xung kích ho?t 10us trên chân TRIG */
    GPIOA->BSRR = (1U << TRIG_PIN);
    uint32_t start = TIM2->CNT;
    while ((TIM2->CNT - start) < 10U)
    {
        /* Ch? block ng?n 10us */
    }
    GPIOA->BSRR = (1U << (TRIG_PIN + 16U));

    s_triggerTime_us = TIM2->CNT;
}

/*------------------------------------------------------------------*/
/* Call from EXTI1_IRQHandler() */
/*------------------------------------------------------------------*/
void HCSR04_ECHO_IRQHandler(void)
{
    if (!(EXTI->PR & (1U << ECHO_PIN)))
    {
        return;
    }

    EXTI->PR = (1U << ECHO_PIN);

    if (GPIOA->IDR & (1U << ECHO_PIN))
    {
        /* Rising edge */
        s_echoRiseTime_us = TIM2->CNT;
    }
    else
    {
        /* Falling edge */
        if (s_measuring)
        {
            uint32_t width_us;
            uint32_t distance_cm10;

            width_us = TIM2->CNT - s_echoRiseTime_us;

            if ((width_us > 0U) && (width_us < ECHO_TIMEOUT_US))
            {
                distance_cm10 = ((width_us * 10U) + 29U) / 58U;

                Rte_Write_RawDistanceCm10(distance_cm10);
                Rte_Write_RawDistance((distance_cm10 + 5U) / 10U);
            }
            else
            {
                Rte_Write_RawDistanceCm10(0U);
                Rte_Write_RawDistance(0U);
            }

            s_measuring = 0U;
        }
    }
}