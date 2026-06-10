#include "Alerts.h"

extern __IO uint32_t uwTick;

static uint32_t last_toggle = 0;
static bool toggle_state = false;

void Alerts_Init(void) {
    /* 1. Enable Clock for GPIOC */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    /* 2. Configure PC0 (Green), PC1 (Yellow), PC2 (Red), and PC3 (Buzzer) as Outputs */
    GPIOC->MODER &= ~((3U << (0 * 2)) | (3U << (1 * 2)) | (3U << (2 * 2)) | (3U << (3 * 2)));
    GPIOC->MODER |= (1U << (0 * 2)) | (1U << (1 * 2)) | (1U << (2 * 2)) | (1U << (3 * 2));

    /* 3. Turn off all LEDs and Buzzer (Active High - pull Low to turn off) */
    GPIOC->BSRR = (1U << (0 + 16)) | (1U << (1 + 16)) | (1U << (2 + 16)) | (1U << (3 + 16));

    last_toggle = 0;
    toggle_state = false;
}

void Alerts_SetSolid(bool green, bool yellow, bool red, bool buzzer) {
    GPIOC->BSRR = green ? (1U << 0) : (1U << (0 + 16));
    GPIOC->BSRR = yellow ? (1U << 1) : (1U << (1 + 16));
    GPIOC->BSRR = red ? (1U << 2) : (1U << (2 + 16));
    GPIOC->BSRR = buzzer ? (1U << 3) : (1U << (3 + 16));
}

void Alerts_UpdateBlink(bool green, bool yellow, bool red, bool buzzer, uint32_t interval_ms) {
    uint32_t current_time = uwTick;
    
    if (current_time - last_toggle >= interval_ms) {
        last_toggle = current_time;
        toggle_state = !toggle_state;
    }
    GPIOC->BSRR = (green && toggle_state) ? (1U << 0) : (1U << (0 + 16));
    GPIOC->BSRR = (yellow && toggle_state) ? (1U << 1) : (1U << (1 + 16));
    GPIOC->BSRR = (red && toggle_state) ? (1U << 2) : (1U << (2 + 16));
    GPIOC->BSRR = (buzzer && toggle_state) ? (1U << 3) : (1U << (3 + 16));
}
