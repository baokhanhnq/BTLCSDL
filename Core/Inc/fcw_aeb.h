/**
  ******************************************************************************
  * @file           : fcw_aeb.h
  * @brief          : Forward Collision Warning and Automatic Emergency Braking
  *                   system framework header.
  ******************************************************************************
  */

#ifndef INC_FCW_AEB_H_
#define INC_FCW_AEB_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported Types ------------------------------------------------------------*/

/**
 * @brief System states representing current vehicle safety and control status.
 */
typedef enum {
    STATE_CRUISE = 0,       /*!< Normal operation: motor follows throttle potentiometer */
    STATE_FCW,              /*!< Forward Collision Warning: LED & Buzzer blink, motor capped or controlled */
    STATE_AEB,              /*!< Automatic Emergency Braking: motor stopped immediately, active braking engaged */
    STATE_SAFE_RELEASE      /*!< Recovery state: motor locked at 0% until throttle is fully released to safe level */
} SystemState_t;

/**
 * @brief System distance and throttle thresholds configuration.
 */
typedef struct {
    uint16_t warning_distance;      /*!< Distance threshold for FCW warning in cm (e.g. 50 cm) */
    uint16_t danger_distance;       /*!< Distance threshold for AEB intervention in cm (e.g. 20 cm) */
    uint16_t safe_throttle_limit;   /*!< Potentiometer safe threshold (out of 4095) for release check (e.g. 400 ~ 10%) */
} SystemConfig_t;

/**
 * @brief Peripheral and GPIO mapping pointers.
 */
typedef struct {
    ADC_HandleTypeDef *hadc_throttle;   /*!< Handle to ADC converting potentiometer value */
    TIM_HandleTypeDef *htim_pwm;        /*!< Handle to Timer for PWM motor speed control */
    uint32_t pwm_channel;               /*!< PWM Channel (e.g., TIM_CHANNEL_1) */
    
    /* HC-SR04 ultrasonic sensor pins */
    GPIO_TypeDef *trig_port;
    uint16_t trig_pin;
    GPIO_TypeDef *echo_port;
    uint16_t echo_pin;
    
    /* Motor Driver L298N direction control pins */
    GPIO_TypeDef *in1_port;
    uint16_t in1_pin;
    GPIO_TypeDef *in2_port;
    uint16_t in2_pin;
    
    /* Warning peripherals */
    GPIO_TypeDef *led_port;
    uint16_t led_pin;
    GPIO_TypeDef *buzzer_port;
    uint16_t buzzer_pin;
} HardwareConfig_t;

/**
 * @brief Main system runtime context structure.
 */
typedef struct {
    SystemState_t current_state;        /*!< Active system state */
    SystemConfig_t config;              /*!< System threshold parameters */
    HardwareConfig_t hw;                /*!< Peripheral configuration mapping */
    
    uint16_t raw_distance;              /*!< Latest raw measured distance in cm */
    uint16_t filtered_distance;         /*!< Latest filtered distance in cm */
    uint16_t filter_buffer[5];          /*!< Median filter circular buffer */
    uint8_t filter_index;               /*!< Median filter circular index */
    uint32_t adc_value;                 /*!< Current raw throttle value (0 - 4095) */
    uint16_t motor_pwm_duty;            /*!< Calculated motor speed duty cycle (0 - 100%) */
    
    uint32_t last_warning_toggle;       /*!< Timestamp for alert toggle (blinking LED/Buzzer) */
    bool warning_toggle_state;          /*!< State of the warning toggle (on/off) */
} FcwAebContext_t;

/* Exported Functions --------------------------------------------------------*/

/**
 * @brief Initializes the FCW & AEB simulation context and sets up starting states.
 * @param ctx Pointer to the FcwAebContext_t context struct
 * @param hw Hardware configurations
 * @param config System threshold configurations
 */
void FCW_AEB_Init(FcwAebContext_t *ctx, HardwareConfig_t hw, SystemConfig_t config);

/**
 * @brief Performs HC-SR04 sensor trigger and calculates the measured raw distance in cm.
 * @param ctx Pointer to context
 * @return Raw measured distance in centimeters (returns 999 if timeout or out of range)
 */
uint16_t FCW_AEB_MeasureDistance(FcwAebContext_t *ctx);

/**
 * @brief Updates the median filtering buffer and returns the median distance.
 * @param ctx Pointer to context
 * @param new_distance Latest raw measurement
 * @return Filtered distance in cm
 */
uint16_t FCW_AEB_MedianFilter(FcwAebContext_t *ctx, uint16_t new_distance);

/**
 * @brief Runs a single step of the FCW/AEB safety logic, transitions state machine, and controls outputs.
 * @param ctx Pointer to context
 */
void FCW_AEB_Process(FcwAebContext_t *ctx);

/**
 * @brief Utility delay function to wait in microseconds.
 * @param us Duration in microseconds
 */
void delay_us(uint16_t us);

#ifdef __cplusplus
}
#endif

#endif /* INC_FCW_AEB_H_ */
