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
    GPIO_TypeDef *green_led_port;       /*!< GPIO port for Green LED (Safe status) */
    uint16_t green_led_pin;             /*!< GPIO pin for Green LED */
    GPIO_TypeDef *yellow_led_port;      /*!< GPIO port for Yellow LED (FCW Warning) */
    uint16_t yellow_led_pin;            /*!< GPIO pin for Yellow LED */
    GPIO_TypeDef *red_led_port;         /*!< GPIO port for Red LED (AEB Brake) */
    uint16_t red_led_pin;               /*!< GPIO pin for Red LED */
    
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
 * @brief Sets warning LEDs and Buzzer to a solid ON or OFF state.
 * @param config Pointer to LedBuzzer_Config_t
 * @param green Safe LED status
 * @param yellow FCW Warning LED status
 * @param red AEB Brake LED status
 * @param buzzer Warning Buzzer status
 */
void LedBuzzer_SetSolid(LedBuzzer_Config_t *config, bool green, bool yellow, bool red, bool buzzer);

/**
 * @brief Performs non-blocking blinking of specified outputs at a specified interval.
 * @param config Pointer to LedBuzzer_Config_t
 * @param green Blinking flag for Green LED
 * @param yellow Blinking flag for Yellow LED
 * @param red Blinking flag for Red LED
 * @param buzzer Blinking flag for Buzzer
 * @param interval_ms Blink interval duration in milliseconds
 */
void LedBuzzer_UpdateBlink(LedBuzzer_Config_t *config, bool green, bool yellow, bool red, bool buzzer, uint32_t interval_ms);

#ifdef __cplusplus
}
#endif

#endif /* INC_LED_BUZZER_H_ */
