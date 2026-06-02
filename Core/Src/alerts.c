/**
  ******************************************************************************
  * @file           : alerts.c
  * @brief          : LED and Buzzer alert warning system implementation.
  ******************************************************************************
  */

#include "alerts.h"

/**
 * @brief Initializes warning outputs to off and clears timestamp.
 * @param config Pointer to Alerts_Config_t
 */
void Alerts_Init(Alerts_Config_t *config) {
    if (config == NULL) return;
    
    config->last_warning_toggle = 0;
    config->warning_toggle_state = false;
    
    // Ensure alert outputs are off initially
    if (config->led_port != NULL) {
        HAL_GPIO_WritePin(config->led_port, config->led_pin, GPIO_PIN_RESET);
    }
    if (config->buzzer_port != NULL) {
        HAL_GPIO_WritePin(config->buzzer_port, config->buzzer_pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief Sets both warning LED and Buzzer to a solid ON or OFF state.
 * @param config Pointer to Alerts_Config_t
 * @param state Target status (true for solid ON, false for solid OFF)
 */
void Alerts_SetSolid(Alerts_Config_t *config, bool state) {
    if (config == NULL) return;
    
    GPIO_PinState pin_state = state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    
    if (config->led_port != NULL) {
        HAL_GPIO_WritePin(config->led_port, config->led_pin, pin_state);
    }
    if (config->buzzer_port != NULL) {
        HAL_GPIO_WritePin(config->buzzer_port, config->buzzer_pin, pin_state);
    }
}

/**
 * @brief Performs non-blocking blinking of both LED and Buzzer at a specified interval.
 * @param config Pointer to Alerts_Config_t
 * @param interval_ms Blink interval duration in milliseconds
 */
void Alerts_UpdateBlink(Alerts_Config_t *config, uint32_t interval_ms) {
    if (config == NULL) return;
    
    uint32_t current_time = HAL_GetTick();
    
    if (current_time - config->last_warning_toggle >= interval_ms) {
        config->last_warning_toggle = current_time;
        config->warning_toggle_state = !config->warning_toggle_state;
        
        GPIO_PinState pin_state = config->warning_toggle_state ? GPIO_PIN_SET : GPIO_PIN_RESET;
        
        if (config->led_port != NULL) {
            HAL_GPIO_WritePin(config->led_port, config->led_pin, pin_state);
        }
        if (config->buzzer_port != NULL) {
            HAL_GPIO_WritePin(config->buzzer_port, config->buzzer_pin, pin_state);
        }
    }
}
