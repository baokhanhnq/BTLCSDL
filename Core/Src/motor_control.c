/**
  ******************************************************************************
  * @file           : motor_control.c
  * @brief          : Motor and Brake control module implementation.
  ******************************************************************************
  */

#include "motor_control.h"
#include "adc.h"

/**
 * @brief Initializes the L298N control state (starts PWM, sets forward direction).
 * @param config Pointer to Motor_Config_t configuration
 */
void Motor_Init(Motor_Config_t *config) {
    if (config == NULL) return;
    
    // Start the PWM timer for L298N Speed ENA
    if (config->htim_pwm != NULL) {
        HAL_TIM_PWM_Start(config->htim_pwm, config->pwm_channel);
    }
    
    // Set default motor direction to Forward (IN1 = HIGH, IN2 = LOW)
    if (config->in1_port != NULL && config->in2_port != NULL) {
        HAL_GPIO_WritePin(config->in1_port, config->in1_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(config->in2_port, config->in2_pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief Reads the raw ADC value from the potentiometer (throttle simulator).
 * @param config Pointer to Motor_Config_t configuration
 * @return Raw ADC reading (0 to 4095)
 */
uint32_t Motor_ReadThrottleAdc(Motor_Config_t *config) {
    if (config == NULL || config->hadc_throttle == NULL) {
        return 0;
    }
    return ADC_Read(config->hadc_throttle);
}

/**
 * @brief Converts raw ADC value (0-4095) to Throttle Duty Cycle percent (0-100%).
 * @param adc_val Raw ADC value
 * @return Throttle Duty Cycle percent (0 to 100)
 */
uint16_t Motor_AdcToDuty(uint32_t adc_val) {
    // Convert ADC (0 - 4095) to Throttle Duty (0% - 100%)
    uint32_t duty = (adc_val * 100) / 4095;
    if (duty > 100) duty = 100;
    return (uint16_t)duty;
}

/**
 * @brief Configures the timer duty cycle for PWM speed control.
 * @param config Pointer to Motor_Config_t configuration
 * @param duty Target speed percentage (0% to 100%)
 */
void Motor_SetSpeed(Motor_Config_t *config, uint16_t duty) {
    if (config == NULL || config->htim_pwm == NULL) return;
    
    // Ensure duty cycle is clamped in 0% - 100%
    if (duty > 100) duty = 100;
    
    // Convert percentage to Timer Auto-Reload Register (ARR) value
    // Duty value = (ARR * duty) / 100
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(config->htim_pwm);
    uint32_t compare_val = (arr * duty) / 100;
    
    __HAL_TIM_SET_COMPARE(config->htim_pwm, config->pwm_channel, compare_val);
}

/**
 * @brief Cuts PWM and applies electric braking to the motor (IN1=LOW, IN2=LOW).
 * @param config Pointer to Motor_Config_t configuration
 */
void Motor_ApplyBrake(Motor_Config_t *config) {
    if (config == NULL) return;
    
    // 1. Cut speed to 0% immediately
    Motor_SetSpeed(config, 0);
    
    // 2. Set active electric braking on L298N (IN1 = LOW, IN2 = LOW)
    // Dynamic braking: setting both inputs to the same logic level locks the motor shafts.
    if (config->in1_port != NULL && config->in2_port != NULL) {
        HAL_GPIO_WritePin(config->in1_port, config->in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(config->in2_port, config->in2_pin, GPIO_PIN_RESET);
    }
}
