/**
  ******************************************************************************
  * @file           : hcsr04.h
  * @brief          : HC-SR04 Ultrasonic Sensor Driver Header.
  ******************************************************************************
  */

#ifndef INC_HCSR04_H_
#define INC_HCSR04_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "delay.h"
#include <stdint.h>

/* Exported Types ------------------------------------------------------------*/

/**
 * @brief HC-SR04 sensor hardware pin mapping configuration structure.
 */
typedef struct {
    GPIO_TypeDef *trig_port;   /*!< GPIO port for Trigger pin */
    uint16_t trig_pin;         /*!< GPIO pin for Trigger */
    GPIO_TypeDef *echo_port;   /*!< GPIO port for Echo pin */
    uint16_t echo_pin;         /*!< GPIO pin for Echo */
} HCSR04_Config_t;

/* Exported Functions --------------------------------------------------------*/

/**
 * @brief Sends a 10us trigger pulse and measures the echo pulse duration to calculate distance.
 * @param config Pointer to HC-SR04 pin configuration
 * @return Measured distance in centimeters (2 to 400 cm). Returns 999 on timeout/error.
 */
uint16_t HCSR04_MeasureDistance(HCSR04_Config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* INC_HCSR04_H_ */
