#include "HCSR04.h"
#include "Rte.h"
#include "stm32f4xx.h"

/* PA0: ngo ra trigger TIM2_CH1. */
#define TRIG_PIN            0U

/* PA1: ngo vao ECHO qua ngat EXTI1. */
#define ECHO_PIN            1U

/* Định nghĩa thời gian Timeout 20 ms */
#define ECHO_TIMEOUT_US     20000U
#define TIM2_PERIOD_US      20000U

/* Các biến tĩnh nội bộ quản lý trạng thái */
static volatile uint32_t s_echoRiseTime_us = 0U;
static volatile uint32_t s_echoFallTime_us = 0U;
static volatile uint8_t  s_measuring       = 0U;
static volatile uint8_t  s_dataReady       = 0U;

/* Bộ đệm lịch sử lưu khoảng cách hợp lệ gần nhất (0.1 cm) */
static uint32_t s_lastDistance_cm10 = 0U;

/* Cấu hình TIM2 CH1 tạo xung trigger 10 us tự động mỗi 20 ms trên PA0 */
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

/* Cấu hình PA0 là chức năng thay thế TIM2_CH1 và PA1 là ngõ vào ECHO */
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

/* Cấu hình EXTI1 BAN ĐẦU CHỈ BẮT SƯỜN LÊN trên PA1 */
static void EXTI1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI1;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PA;

    EXTI->RTSR |= (1U << ECHO_PIN);  /* Bắt sườn lên */
    EXTI->FTSR &= ~(1U << ECHO_PIN); /* Tắt sườn xuống */
    EXTI->IMR  |= (1U << ECHO_PIN);
    EXTI->PR    = (1U << ECHO_PIN);

    NVIC_SetPriority(EXTI1_IRQn, 1U);
    NVIC_EnableIRQ(EXTI1_IRQn);
}

/* 1. HÀM INIT: Khởi tạo tất cả giá trị ban đầu                     */
void HCSR04_Init(void)
{
    GPIO_HCSR04_Init();
    TIM2_PWM_Init();
    EXTI1_Init();

    s_lastDistance_cm10      = 0U;
    s_echoRiseTime_us        = 0U;
    s_echoFallTime_us        = 0U;
    s_measuring              = 0U;
    s_dataReady              = 0U;
}

/* 2. HÀM PROCESS: Gánh toàn bộ phép tính trừ thời gian và quy đổi */
/* Chạy định kỳ 20ms một lần                                        */
void HCSR04_Process(void)
{
    /* Mặc định lấy kết quả hợp lệ trước đó */
    uint32_t current_distance_cm10 = s_lastDistance_cm10;

    if (s_measuring != 0U)
    {
        s_measuring = 0U;
        s_dataReady = 0U;

        /* Ép phần cứng EXTI quay về trạng thái bắt sườn lên */
        EXTI->RTSR |= (1U << ECHO_PIN);
        EXTI->FTSR &= ~(1U << ECHO_PIN);
    }
    /* KIỂM TRA ĐIỀU KIỆN: Đo thành công (Sườn xuống hạ kịp thời trước 20ms) */
    else if (s_dataReady != 0U)
    {
        s_dataReady = 0U;

        uint32_t rise = s_echoRiseTime_us;
        uint32_t fall = s_echoFallTime_us;
        uint32_t width_us = 0U;

        if (fall >= rise)
        {
            width_us = fall - rise;
        }
        else
        {
            width_us = (TIM2_PERIOD_US - rise) + fall;
        }

        /* Tiến hành tính toán quy đổi ra khoảng cách vật lý */
        if ((width_us > 0U) && (width_us < ECHO_TIMEOUT_US))
        {
            current_distance_cm10 = ((width_us * 10U) + 29U) / 58U;
            s_lastDistance_cm10   = current_distance_cm10;
        }
    }

    /* GHI GIÁ TRỊ CUỐI CÙNG LÊN CẢ HAI CỔNG RTE */
    Rte_Write_RawDistanceCm10((uint16_t)current_distance_cm10);
    Rte_Write_RawDistance((uint16_t)((current_distance_cm10 + 5U) / 10U));

    /* BẮT ĐẦU CHU KỲ QUÉT MỚI */
    EXTI->PR    = (1U << ECHO_PIN);
    s_measuring = 1U;
}

/*Interupt */
void HCSR04_ECHO_IRQHandler(void)
{
    if ((EXTI->PR & (1U << ECHO_PIN)) == 0U)
    {
        return;
    }
    EXTI->PR = (1U << ECHO_PIN); /* Xóa cờ ngắt */

    uint32_t timestamp = TIM2->CNT; /* Chụp mốc thời gian phần cứng */

    if ((EXTI->RTSR & (1U << ECHO_PIN)) != 0U)
    {
        s_echoRiseTime_us = timestamp; /* Chỉ lưu mốc sườn lên */

        /*Đảo chiều */
        EXTI->RTSR &= ~(1U << ECHO_PIN);
        EXTI->FTSR |= (1U << ECHO_PIN);
        return;
    }
    else if ((EXTI->FTSR & (1U << ECHO_PIN)) != 0U)
    {
        if (s_measuring != 0U)
        {
            s_echoFallTime_us = timestamp; /* Chụp dữ liệu*/
            s_dataReady       = 1U;        /* Bật cờ báo có dữ liệu thô */
            s_measuring       = 0U;        /* Hạ cờ bận */
        }

        /* Đảo chiều quay lại: Tắt bắt sườn xuống, bật lại bắt sườn lên đón chu kỳ sau */
        EXTI->FTSR &= ~(1U << ECHO_PIN);
        EXTI->RTSR |= (1U << ECHO_PIN);
    }
}
