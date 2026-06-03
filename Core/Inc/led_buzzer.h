/**
  ******************************************************************************
  * @file           : led_buzzer.h
  * @brief          : LED and Buzzer warning system header.
  ******************************************************************************
  */

#ifndef INC_LED_BUZZER_H_
#define INC_LED_BUZZER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported Types ------------------------------------------------------------*/

/**
 * @brief LED & Buzzer warning hardware configuration and toggle state.
 */
typedef struct {
    GPIO_TypeDef *led_port;             /*!< GPIO port for warning LED */
    uint16_t led_pin;                   /*!< GPIO pin for warning LED */
    GPIO_TypeDef *buzzer_port;          /*!< GPIO port for warning Buzzer */
    uint16_t buzzer_pin;                /*!< GPIO pin for warning Buzzer */
    
    uint32_t last_warning_toggle;       /*!< Timestamp for alert toggle (blinking LED/Buzzer) */
    bool warning_toggle_state;          /*!< State of the warning toggle (on/off) */
} LedBuzzer_Config_t;

/* Exported Functions --------------------------------------------------------*/

/**
 * @brief Initializes warning outputs to off.
 * @param config Pointer to LedBuzzer_Config_t
 */
void LedBuzzer_Init(LedBuzzer_Config_t *config);

/**
 * @brief Sets both warning LED and Buzzer to a solid ON or OFF state.
 * @param config Pointer to LedBuzzer_Config_t
 * @param state Target status (true for solid ON, false for solid OFF)
 */
void LedBuzzer_SetSolid(LedBuzzer_Config_t *config, bool state);

/**
 * @brief Performs non-blocking blinking of both LED and Buzzer at a specified interval.
 * @param config Pointer to LedBuzzer_Config_t
 * @param interval_ms Blink interval duration in milliseconds
 */
void LedBuzzer_UpdateBlink(LedBuzzer_Config_t *config, uint32_t interval_ms);

#ifdef __cplusplus
}
#endif

#endif /* INC_LED_BUZZER_H_ */
