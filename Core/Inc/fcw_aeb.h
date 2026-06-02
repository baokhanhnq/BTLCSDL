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
#include "hcsr04.h"
#include "filter.h"
#include "motor_control.h"
#include "alerts.h"
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
 * @brief Main system runtime context structure integrating all sub-module handlers.
 */
typedef struct {
    SystemState_t current_state;        /*!< Active system state */
    SystemConfig_t config;              /*!< System threshold parameters */
    
    Motor_Config_t motor;              /*!< Motor and throttle driver configuration */
    HCSR04_Config_t sensor;            /*!< Ultrasonic sensor configuration */
    Alerts_Config_t alerts;            /*!< Alert outputs warning system configuration */
    MedianFilter_t filter;             /*!< Median filtering circular buffer */
    
    uint16_t raw_distance;              /*!< Latest raw measured distance in cm */
    uint16_t filtered_distance;         /*!< Latest filtered distance in cm */
    uint32_t adc_value;                 /*!< Current raw throttle value (0 - 4095) */
    uint16_t motor_pwm_duty;            /*!< Calculated motor speed duty cycle (0 - 100%) */
} FcwAebContext_t;

/* Exported Functions --------------------------------------------------------*/

/**
 * @brief Initializes the FCW & AEB simulation context and sets up starting states.
 * @param ctx Pointer to the FcwAebContext_t context struct
 * @param motor Configured Motor_Config_t struct
 * @param sensor Configured HCSR04_Config_t struct
 * @param alerts Configured Alerts_Config_t struct
 * @param config System threshold configurations
 */
void FCW_AEB_Init(FcwAebContext_t *ctx, Motor_Config_t motor, HCSR04_Config_t sensor, Alerts_Config_t alerts, SystemConfig_t config);

/**
 * @brief Runs a single step of the FCW/AEB safety logic, transitions state machine, and controls outputs.
 * @param ctx Pointer to context
 */
void FCW_AEB_Process(FcwAebContext_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* INC_FCW_AEB_H_ */
