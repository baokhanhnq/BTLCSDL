// Test
#include "HCSR04.h"
#include "Rte.h"

/* PA0 -> TRIG */
#define TRIG_PIN                    0U
#define TRIG_MASK                   (1U << TRIG_PIN)

/* PA1 -> ECHO */
#define ECHO_PIN                    1U
#define ECHO_MASK                   (1U << ECHO_PIN)

/* HC-SR04 timing */
#define TRIGGER_PULSE_US            10U
#define ECHO_TIMEOUT_US             30000U

/* HCSR04_Process() is expected to run every 10ms. Trigger every 50ms. */
#define TRIGGER_PERIOD_TASK_TICKS   5U

static volatile uint32_t s_triggerTime_us = 0U;
static volatile uint32_t s_echoRiseTime_us = 0U;
static volatile uint16_t s_lastDistance_cm10 = 0U;
static volatile uint8_t s_measuring = 0U;
static volatile uint8_t s_echoRiseCaptured = 0U;

static uint8_t s_triggerCounter = 0U;

/*------------------------------------------------------------------*/
/* TIM2 */
/*------------------------------------------------------------------*/
static uint32_t HCSR04_GetTim2ClockHz(void)
{
    uint32_t hclk;
    uint32_t ppre1;
    uint32_t apb1_div;
    uint32_t pclk1;

    SystemCoreClockUpdate();
    hclk = SystemCoreClock;
    ppre1 = (RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos;

    if (ppre1 < 4U)
    {
        apb1_div = 1U;
    }
    else
    {
        apb1_div = 1U << (ppre1 - 3U);
    }

    pclk1 = hclk / apb1_div;
    return (apb1_div == 1U) ? pclk1 : (pclk1 * 2U);
}

static void TIM2_Init(void)
{
    uint32_t tim2_clock_hz;
    uint32_t prescaler;

    tim2_clock_hz = HCSR04_GetTim2ClockHz();
    prescaler = tim2_clock_hz / 1000000U;
    if (prescaler == 0U)
    {
        prescaler = 1U;
    }

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->CR1 = 0U;
    TIM2->PSC = prescaler - 1U;
    TIM2->ARR = 0xFFFFFFFFU;
    TIM2->CNT = 0U;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 = TIM_CR1_CEN;
}

/*------------------------------------------------------------------*/
/* GPIO */
/*------------------------------------------------------------------*/
static void GPIO_HCSR04_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* PA0 output: TRIG */
    GPIOA->MODER &= ~(3U << (TRIG_PIN * 2U));
    GPIOA->MODER |=  (1U << (TRIG_PIN * 2U));
    GPIOA->OTYPER &= ~TRIG_MASK;
    GPIOA->PUPDR  &= ~(3U << (TRIG_PIN * 2U));
    GPIOA->BSRR = (TRIG_MASK << 16U);

    /* PA1 input: ECHO */
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

    EXTI->RTSR |= ECHO_MASK;
    EXTI->FTSR |= ECHO_MASK;
    EXTI->IMR  |= ECHO_MASK;
    EXTI->PR = ECHO_MASK;

    NVIC_SetPriority(EXTI1_IRQn, 1U);
    NVIC_EnableIRQ(EXTI1_IRQn);
}

/*------------------------------------------------------------------*/
/* Distance */
/*------------------------------------------------------------------*/
static uint16_t HCSR04_ConvertPulseToCm10(uint32_t width_us)
{
    if ((width_us == 0U) || (width_us >= ECHO_TIMEOUT_US))
    {
        return 0U;
    }

    return (uint16_t)(((width_us * 10U) + 29U) / 58U);
}

static void HCSR04_WriteDistance(uint16_t distance_cm10)
{
    s_lastDistance_cm10 = distance_cm10;
    Rte_Write_RawDistanceCm10(distance_cm10);
    Rte_Write_RawDistance((uint16_t)((distance_cm10 + 5U) / 10U));
}

/*------------------------------------------------------------------*/
/* Measurement */
/*------------------------------------------------------------------*/
static void HCSR04_ResetMeasurement(void)
{
    s_triggerTime_us = 0U;
    s_echoRiseTime_us = 0U;
    s_measuring = 0U;
    s_echoRiseCaptured = 0U;
}

static void HCSR04_SendTriggerPulse(void)
{
    uint32_t start_us;

    GPIOA->BSRR = TRIG_MASK;
    start_us = TIM2->CNT;

    while ((TIM2->CNT - start_us) < TRIGGER_PULSE_US)
    {
        /* Keep TRIG high for 10us. */
    }

    GPIOA->BSRR = (TRIG_MASK << 16U);
}

static void HCSR04_StartMeasurement(void)
{
    EXTI->PR = ECHO_MASK;

    s_echoRiseCaptured = 0U;
    s_measuring = 1U;

    HCSR04_SendTriggerPulse();
    s_triggerTime_us = TIM2->CNT;
}

static void HCSR04_CheckTimeout(void)
{
    uint32_t elapsed_us;

    if (s_measuring == 0U)
    {
        return;
    }

    elapsed_us = TIM2->CNT - s_triggerTime_us;
    if (elapsed_us < ECHO_TIMEOUT_US)
    {
        return;
    }

    HCSR04_ResetMeasurement();
    HCSR04_WriteDistance(0U);
}

/*------------------------------------------------------------------*/
/* Init */
/*------------------------------------------------------------------*/
void HCSR04_Init(void)
{
    TIM2_Init();
    GPIO_HCSR04_Init();
    EXTI1_Init();

    HCSR04_ResetMeasurement();
    s_lastDistance_cm10 = 0U;
    s_triggerCounter = 0U;
}

/*------------------------------------------------------------------*/
/* Cyclic process */
/*------------------------------------------------------------------*/
void HCSR04_Process(void)
{
    if (s_measuring != 0U)
    {
        HCSR04_CheckTimeout();
        return;
    }

    s_triggerCounter++;
    if (s_triggerCounter < TRIGGER_PERIOD_TASK_TICKS)
    {
        return;
    }

    s_triggerCounter = 0U;
    HCSR04_StartMeasurement();
}

/*------------------------------------------------------------------*/
/* Call from EXTI1_IRQHandler() */
/*------------------------------------------------------------------*/
void HCSR04_EchoIrqHandler(void)
{
    uint32_t width_us;
    uint16_t distance_cm10;

    if ((EXTI->PR & ECHO_MASK) == 0U)
    {
        return;
    }

    EXTI->PR = ECHO_MASK;

    if ((GPIOA->IDR & ECHO_MASK) != 0U)
    {
        /* Rising edge */
        s_echoRiseTime_us = TIM2->CNT;
        s_echoRiseCaptured = 1U;
        return;
    }

    /* Falling edge */
    if ((s_measuring == 0U) || (s_echoRiseCaptured == 0U))
    {
        return;
    }

    width_us = TIM2->CNT - s_echoRiseTime_us;
    distance_cm10 = HCSR04_ConvertPulseToCm10(width_us);

    HCSR04_WriteDistance(distance_cm10);
    HCSR04_ResetMeasurement();
}

/*------------------------------------------------------------------*/
/* Compatibility wrappers */
/*------------------------------------------------------------------*/
void Hcsr04_GetDistance(void)
{
    HCSR04_Process();
}

void HCSR04_ECHO_IRQHandler(void)
{
    HCSR04_EchoIrqHandler();
}

uint32_t HCSR04_ReadCm10(void)
{
    return s_lastDistance_cm10;
}

uint32_t HCSR04_Read(void)
{
    uint32_t distance_cm10 = HCSR04_ReadCm10();
    return (distance_cm10 + 5U) / 10U;
}
