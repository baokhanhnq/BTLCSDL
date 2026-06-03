/**
  ******************************************************************************
  * @file           : led_buzzer.c
  * @brief          : LED and Buzzer warning system implementation.
  ******************************************************************************
  */

#include "led_buzzer.h"

/**
 * @brief Initializes warning outputs to off and clears timestamp.
 * @param config Pointer to LedBuzzer_Config_t
 */
void LedBuzzer_Init(LedBuzzer_Config_t *config) {
    if (config == NULL) return;
    
    config->last_warning_toggle = 0;
    config->warning_toggle_state = false;
    
    // Đưa tất cả đầu ra về mức thấp (tắt) khi khởi tạo
    if (config->green_led_port != NULL) {
        HAL_GPIO_WritePin(config->green_led_port, config->green_led_pin, GPIO_PIN_RESET);
    }
    if (config->yellow_led_port != NULL) {
        HAL_GPIO_WritePin(config->yellow_led_port, config->yellow_led_pin, GPIO_PIN_RESET);
    }
    if (config->red_led_port != NULL) {
        HAL_GPIO_WritePin(config->red_led_port, config->red_led_pin, GPIO_PIN_RESET);
    }
    if (config->buzzer_port != NULL) {
        HAL_GPIO_WritePin(config->buzzer_port, config->buzzer_pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief Sets warning LEDs and Buzzer to a solid ON or OFF state.
 * @param config Pointer to LedBuzzer_Config_t
 * @param green Safe LED status
 * @param yellow FCW Warning LED status
 * @param red AEB Brake LED status
 * @param buzzer Warning Buzzer status
 */
void LedBuzzer_SetSolid(LedBuzzer_Config_t *config, bool green, bool yellow, bool red, bool buzzer) {
    if (config == NULL) return;
    
    if (config->green_led_port != NULL) {
        HAL_GPIO_WritePin(config->green_led_port, config->green_led_pin, green ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
    if (config->yellow_led_port != NULL) {
        HAL_GPIO_WritePin(config->yellow_led_port, config->yellow_led_pin, yellow ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
    if (config->red_led_port != NULL) {
        HAL_GPIO_WritePin(config->red_led_port, config->red_led_pin, red ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
    if (config->buzzer_port != NULL) {
        HAL_GPIO_WritePin(config->buzzer_port, config->buzzer_pin, buzzer ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

/**
 * @brief Performs non-blocking blinking of specified outputs at a specified interval.
 * @param config Pointer to LedBuzzer_Config_t
 * @param green Blinking flag for Green LED
 * @param yellow Blinking flag for Yellow LED
 * @param red Blinking flag for Red LED
 * @param buzzer Blinking flag for Buzzer
 * @param interval_ms Blink interval duration in milliseconds
 */
void LedBuzzer_UpdateBlink(LedBuzzer_Config_t *config, bool green, bool yellow, bool red, bool buzzer, uint32_t interval_ms) {
    if (config == NULL) return;
    
    uint32_t current_time = HAL_GetTick();
    
    if (current_time - config->last_warning_toggle >= interval_ms) {
        config->last_warning_toggle = current_time;
        config->warning_toggle_state = !config->warning_toggle_state;
    }
    
    GPIO_PinState toggle_pin_state = config->warning_toggle_state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    
    if (config->green_led_port != NULL) {
        HAL_GPIO_WritePin(config->green_led_port, config->green_led_pin, green ? toggle_pin_state : GPIO_PIN_RESET);
    }
    if (config->yellow_led_port != NULL) {
        HAL_GPIO_WritePin(config->yellow_led_port, config->yellow_led_pin, yellow ? toggle_pin_state : GPIO_PIN_RESET);
    }
    if (config->red_led_port != NULL) {
        HAL_GPIO_WritePin(config->red_led_port, config->red_led_pin, red ? toggle_pin_state : GPIO_PIN_RESET);
    }
    if (config->buzzer_port != NULL) {
        HAL_GPIO_WritePin(config->buzzer_port, config->buzzer_pin, buzzer ? toggle_pin_state : GPIO_PIN_RESET);
    }
}
