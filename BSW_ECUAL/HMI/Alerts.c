#include "Alerts.h"
#include "Rte.h"

extern __IO uint32_t uwTick;

static uint32_t s_lastToggle_ms = 0U;
static bool s_toggleState = false;

static void Alerts_SetSolid(bool green, bool yellow, bool red, bool buzzer);
static void Alerts_UpdateBlink(bool green,
                               bool yellow,
                               bool red,
                               bool buzzer,
                               uint32_t interval_ms);

/*
 * Khoi tao PC0, PC1, PC2 va PC3 lam ngo ra canh bao.
 */
void Alerts_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    GPIOC->MODER &= ~((3U << (0U * 2U)) |
                      (3U << (1U * 2U)) |
                      (3U << (2U * 2U)) |
                      (3U << (3U * 2U)));
    GPIOC->MODER |= ((1U << (0U * 2U)) |
                     (1U << (1U * 2U)) |
                     (1U << (2U * 2U)) |
                     (1U << (3U * 2U)));

    Alerts_SetSolid(false, false, false, false);

    s_lastToggle_ms = 0U;
    s_toggleState = false;
}

/*
 * Dieu khien LED va buzzer theo trang thai bat/tat co dinh.
 */
static void Alerts_SetSolid(bool green, bool yellow, bool red, bool buzzer)
{
    GPIOC->BSRR = green ? (1U << 0U) : (1U << (0U + 16U));
    GPIOC->BSRR = yellow ? (1U << 1U) : (1U << (1U + 16U));
    GPIOC->BSRR = red ? (1U << 2U) : (1U << (2U + 16U));
    GPIOC->BSRR = buzzer ? (1U << 3U) : (1U << (3U + 16U));
}

/*
 * Nhay cac ngo ra duoc chon theo cung mot chu ky.
 */
static void Alerts_UpdateBlink(bool green,
                               bool yellow,
                               bool red,
                               bool buzzer,
                               uint32_t interval_ms)
{
    uint32_t current_time;

    current_time = uwTick;

    if ((uint32_t)(current_time - s_lastToggle_ms) >= interval_ms)
    {
        s_lastToggle_ms = current_time;
        s_toggleState = !s_toggleState;
    }

    GPIOC->BSRR = (green && s_toggleState) ? (1U << 0U) : (1U << (0U + 16U));
    GPIOC->BSRR = (yellow && s_toggleState) ? (1U << 1U) : (1U << (1U + 16U));
    GPIOC->BSRR = (red && s_toggleState) ? (1U << 2U) : (1U << (2U + 16U));
    GPIOC->BSRR = (buzzer && s_toggleState) ? (1U << 3U) : (1U << (3U + 16U));
}

/*
 * Ham thuc thi canh bao.
 * Doc trang thai he thong tu RTE va cap nhat LED/buzzer.
 */
void Alert_Execute(void)
{
    SystemState_t state = STATE_CRUISE;

    state = Rte_Read_SystemState();

    switch (state)
    {
        case STATE_CRUISE:
            Alerts_SetSolid(true, false, false, false);
            break;

        case STATE_FCW:
            Alerts_UpdateBlink(false, true, false, true, 250U);
            break;

        case STATE_AEB:
            Alerts_SetSolid(false, false, true, true);
            break;

        case STATE_SAFE_RELEASE:
            Alerts_UpdateBlink(false, false, true, true, 100U);
            break;

        default:
            Alerts_SetSolid(false, false, false, false);
            break;
    }
}
