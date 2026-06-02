/**
  ******************************************************************************
  * @file           : hcsr04.c
  * @brief          : HC-SR04 Ultrasonic Sensor Driver Implementation.
  ******************************************************************************
  */

#include "hcsr04.h"

/**
 * @brief Sends a 10us trigger pulse and measures the echo pulse duration to calculate distance.
 * @param config Pointer to HC-SR04 pin configuration
 * @return Measured distance in centimeters. Returns 999 on timeout/error.
 */
uint16_t HCSR04_MeasureDistance(HCSR04_Config_t *config) {
    if (config->trig_port == NULL || config->echo_port == NULL) {
        return 999;
    }
    
    // 1. Send 10us high pulse to Trigger pin
    HAL_GPIO_WritePin(config->trig_port, config->trig_pin, GPIO_PIN_RESET);
    delay_us(2);
    HAL_GPIO_WritePin(config->trig_port, config->trig_pin, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(config->trig_port, config->trig_pin, GPIO_PIN_RESET);
    
    // 2. Measure Echo Pulse Width (Time of Flight)
    // Timeout limits to avoid infinite looping (approx 30ms or ~50000 loops)
    uint32_t timeout_cnt = 0;
    const uint32_t MAX_TIMEOUT = 50000;
    
    // Wait for Echo to go HIGH
    while (HAL_GPIO_ReadPin(config->echo_port, config->echo_pin) == GPIO_PIN_RESET) {
        timeout_cnt++;
        if (timeout_cnt > MAX_TIMEOUT) {
            return 999; // Sensor timed out / disconnected
        }
    }
    
    // Record start cycle count
    uint32_t start_cycles = DWT->CYCCNT;
    timeout_cnt = 0;
    
    // Wait for Echo to go LOW
    while (HAL_GPIO_ReadPin(config->echo_port, config->echo_pin) == GPIO_PIN_SET) {
        timeout_cnt++;
        if (timeout_cnt > MAX_TIMEOUT) {
            return 999; // Pulse took too long (no reflection or sensor error)
        }
    }
    
    // Record end cycle count
    uint32_t end_cycles = DWT->CYCCNT;
    
    // Calculate elapsed ticks
    uint32_t elapsed_ticks = end_cycles - start_cycles;
    
    // Convert ticks to microseconds
    uint32_t pulse_duration_us = elapsed_ticks / (SystemCoreClock / 1000000);
    
    // Calculate distance in cm (duration / 58)
    uint16_t dist = (uint16_t)(pulse_duration_us / 58);
    
    // Filter unrealistic values (HC-SR04 ideal range: 2cm to 400cm)
    if (dist < 2 || dist > 400) {
        return 999;
    }
    
    return dist;
}
