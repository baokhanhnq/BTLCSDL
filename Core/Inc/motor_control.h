/**
  ******************************************************************************
  * @file           : motor_control.h
  * @brief          : Motor and Brake control module header.
  ******************************************************************************
  */

#ifndef INC_MOTOR_CONTROL_H_
#define INC_MOTOR_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>

/* Exported Types ------------------------------------------------------------*/

/**
 * @brief L298N motor driver & Potentiometer ADC configuration.
 */
typedef struct {
    ADC_HandleTypeDef *hadc_throttle;   /*!< Handle to ADC converting potentiometer value */
    TIM_HandleTypeDef *htim_pwm;        /*!< Handle to Timer for PWM motor speed control */
    uint32_t pwm_channel;               /*!< PWM Channel (e.g., TIM_CHANNEL_1) */
    
    /* Motor Driver L298N direction control pins */
    GPIO_TypeDef *in1_port;             /*!< Direction control port IN1 */
    uint16_t in1_pin;                   /*!< Direction control pin IN1 */
    GPIO_TypeDef *in2_port;             /*!< Direction control port IN2 */
    uint16_t in2_pin;                   /*!< Direction control pin IN2 */
} Motor_Config_t;

/* Exported Functions --------------------------------------------------------*/

/**
 * @brief Initializes the L298N control state (starts PWM, sets forward direction).
 * @param config Pointer to Motor_Config_t configuration
 */
void Motor_Init(Motor_Config_t *config);

/**
 * @brief Reads the raw ADC value from the potentiometer (throttle simulator).
 * @param config Pointer to Motor_Config_t configuration
 * @return Raw ADC reading (0 to 4095)
 */
uint32_t Motor_ReadThrottleAdc(Motor_Config_t *config);

/**
 * @brief Configures the timer duty cycle for PWM speed control.
 * @param config Pointer to Motor_Config_t configuration
 * @param duty Target speed percentage (0% to 100%)
 */
void Motor_SetSpeed(Motor_Config_t *config, uint16_t duty);

/**
 * @brief Cuts PWM and applies electric braking to the motor (IN1=LOW, IN2=LOW).
 * @param config Pointer to Motor_Config_t configuration
 */
void Motor_ApplyBrake(Motor_Config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* INC_MOTOR_CONTROL_H_ */
