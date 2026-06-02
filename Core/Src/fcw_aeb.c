/**
  ******************************************************************************
  * @file           : fcw_aeb.c
  * @brief          : Forward Collision Warning and Automatic Emergency Braking
  *                   system implementation.
  ******************************************************************************
  */

#include "fcw_aeb.h"
#include <string.h>

/* Private function prototypes -----------------------------------------------*/
static void DWT_Init(void);
static void FCW_AEB_UpdateAlerts(FcwAebContext_t *ctx);
static void FCW_AEB_SetMotorSpeed(FcwAebContext_t *ctx, uint16_t duty);
static void FCW_AEB_ApplyBrakes(FcwAebContext_t *ctx);

/**
 * @brief Initializes the DWT (Data Watchpoint and Trace) unit for microsecond timing.
 *        This unit is present on Cortex-M4 cores (like STM32F4) and enables
 *        highly precise delay and duration measurements without using extra timers.
 */
static void DWT_Init(void) {
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/**
 * @brief Cycle-accurate microsecond delay using DWT.
 * @param us Microseconds to delay
 */
void delay_us(uint16_t us) {
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = (uint32_t)us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - startTick) < delayTicks) {
        // Wait cycle
    }
}

/**
 * @brief Initializes the FCW & AEB simulation context.
 * @param ctx Pointer to context
 * @param hw Hardware configurations
 * @param config System thresholds
 */
void FCW_AEB_Init(FcwAebContext_t *ctx, HardwareConfig_t hw, SystemConfig_t config) {
    // Clear context
    memset(ctx, 0, sizeof(FcwAebContext_t));
    
    // Copy configurations
    ctx->hw = hw;
    ctx->config = config;
    
    // Set default initial values
    ctx->current_state = STATE_CRUISE;
    ctx->raw_distance = 100;
    ctx->filtered_distance = 100;
    
    // Initialize the filter buffer with safe values (100cm)
    for (int i = 0; i < 5; i++) {
        ctx->filter_buffer[i] = 100;
    }
    ctx->filter_index = 0;
    
    // Initialize timing hardware
    DWT_Init();
    
    // Start the PWM timer for L298N Speed ENA
    if (ctx->hw.htim_pwm != NULL) {
        HAL_TIM_PWM_Start(ctx->hw.htim_pwm, ctx->hw.pwm_channel);
    }
    
    // Set default motor direction to Forward (IN1 = HIGH, IN2 = LOW)
    if (ctx->hw.in1_port != NULL && ctx->hw.in2_port != NULL) {
        HAL_GPIO_WritePin(ctx->hw.in1_port, ctx->hw.in1_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(ctx->hw.in2_port, ctx->hw.in2_pin, GPIO_PIN_RESET);
    }
    
    // Ensure alert outputs are initialized to off
    if (ctx->hw.led_port != NULL) {
        HAL_GPIO_WritePin(ctx->hw.led_port, ctx->hw.led_pin, GPIO_PIN_RESET);
    }
    if (ctx->hw.buzzer_port != NULL) {
        HAL_GPIO_WritePin(ctx->hw.buzzer_port, ctx->hw.buzzer_pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief Triggers the HC-SR04 sensor and calculates the measured raw distance in cm.
 *        Includes safe timeout loops to prevent CPU locking if the sensor is detached.
 * @param ctx Pointer to context
 * @return Distance in cm, or 999 if out of range or sensor disconnected.
 */
uint16_t FCW_AEB_MeasureDistance(FcwAebContext_t *ctx) {
    if (ctx->hw.trig_port == NULL || ctx->hw.echo_port == NULL) {
        return 999;
    }
    
    // 1. Send 10us high pulse to Trigger pin
    HAL_GPIO_WritePin(ctx->hw.trig_port, ctx->hw.trig_pin, GPIO_PIN_RESET);
    delay_us(2);
    HAL_GPIO_WritePin(ctx->hw.trig_port, ctx->hw.trig_pin, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(ctx->hw.trig_port, ctx->hw.trig_pin, GPIO_PIN_RESET);
    
    // 2. Measure Echo Pulse Width (Time of Flight)
    // Timeout limits to avoid infinite looping (approx 30ms or ~50000 loops)
    uint32_t timeout_cnt = 0;
    const uint32_t MAX_TIMEOUT = 50000;
    
    // Wait for Echo to go HIGH
    while (HAL_GPIO_ReadPin(ctx->hw.echo_port, ctx->hw.echo_pin) == GPIO_PIN_RESET) {
        timeout_cnt++;
        if (timeout_cnt > MAX_TIMEOUT) {
            return 999; // Sensor timed out / disconnected
        }
    }
    
    // Record start cycle count
    uint32_t start_cycles = DWT->CYCCNT;
    timeout_cnt = 0;
    
    // Wait for Echo to go LOW
    while (HAL_GPIO_ReadPin(ctx->hw.echo_port, ctx->hw.echo_pin) == GPIO_PIN_SET) {
        timeout_cnt++;
        if (timeout_cnt > MAX_TIMEOUT) {
            return 999; // Pulse took too long (no reflection or sensor error)
        }
    }
    
    // Record end cycle count
    uint32_t end_cycles = DWT->CYCCNT;
    
    // Calculate elapsed ticks (handling potential timer overflow naturally)
    uint32_t elapsed_ticks = end_cycles - start_cycles;
    
    // Convert ticks to microseconds
    uint32_t pulse_duration_us = elapsed_ticks / (SystemCoreClock / 1000000);
    
    // Calculate distance: d = (t * speed of sound) / 2
    // Speed of sound = 343 m/s = 0.0343 cm/us
    // Distance (cm) = (pulse_duration_us * 0.0343) / 2 = pulse_duration_us / 58
    uint16_t dist = (uint16_t)(pulse_duration_us / 58);
    
    // Filter unrealistic values (HC-SR04 ideal range: 2cm to 400cm)
    if (dist < 2 || dist > 400) {
        return 999;
    }
    
    ctx->raw_distance = dist;
    return dist;
}

/**
 * @brief Smooths sensor readings using a rolling Median Filter of size 5.
 * @param ctx Pointer to context
 * @param new_distance Latest raw measurement
 * @return Filtered distance in cm
 */
uint16_t FCW_AEB_MedianFilter(FcwAebContext_t *ctx, uint16_t new_distance) {
    // Insert into circular buffer
    ctx->filter_buffer[ctx->filter_index] = new_distance;
    ctx->filter_index = (ctx->filter_index + 1) % 5;
    
    // Copy the buffer for sorting
    uint16_t sorted[5];
    memcpy(sorted, ctx->filter_buffer, sizeof(sorted));
    
    // Sort array in ascending order (simple Bubble Sort for 5 elements)
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (sorted[i] > sorted[j]) {
                uint16_t temp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = temp;
            }
        }
    }
    
    // Extract the median element
    ctx->filtered_distance = sorted[2];
    return ctx->filtered_distance;
}

/**
 * @brief Processes the main safety loop: reads throttle, updates state, controls outputs.
 * @param ctx Pointer to context
 */
void FCW_AEB_Process(FcwAebContext_t *ctx) {
    // 1. Read throttle value from ADC (simulate accelerator pedal)
    if (ctx->hw.hadc_throttle != NULL) {
        HAL_ADC_Start(ctx->hw.hadc_throttle);
        if (HAL_ADC_PollForConversion(ctx->hw.hadc_throttle, 10) == HAL_OK) {
            ctx->adc_value = HAL_ADC_GetValue(ctx->hw.hadc_throttle);
        }
        HAL_ADC_Stop(ctx->hw.hadc_throttle);
    }
    
    // 2. Convert ADC (0 - 4095) to Throttle Duty (0% - 100%)
    uint16_t target_duty = (uint16_t)((ctx->adc_value * 100) / 4095);
    
    // 3. Measure and filter the obstacle distance
    uint16_t raw_dist = FCW_AEB_MeasureDistance(ctx);
    uint16_t filtered_dist = FCW_AEB_MedianFilter(ctx, raw_dist);
    
    // Define a small hysteresis (e.g. 5cm) to prevent rapid flickering between states
    const uint16_t HYSTERESIS = 5;
    
    // 4. State Machine transitions and action triggers
    switch (ctx->current_state) {
        
        case STATE_CRUISE:
            // Warnings are off in normal mode
            HAL_GPIO_WritePin(ctx->hw.led_port, ctx->hw.led_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(ctx->hw.buzzer_port, ctx->hw.buzzer_pin, GPIO_PIN_RESET);
            
            // Set motor speed following throttle directly
            FCW_AEB_SetMotorSpeed(ctx, target_duty);
            
            // Transition conditions
            if (filtered_dist < ctx->config.danger_distance) {
                ctx->current_state = STATE_AEB;
            } else if (filtered_dist < ctx->config.warning_distance) {
                ctx->current_state = STATE_FCW;
            }
            break;
            
        case STATE_FCW:
            // Drive warning alerts (blink LED & Buzzer)
            FCW_AEB_UpdateAlerts(ctx);
            
            // Safety Option: Limit speed to max 50% in Warning zone
            uint16_t capped_duty = (target_duty > 50) ? 50 : target_duty;
            FCW_AEB_SetMotorSpeed(ctx, capped_duty);
            
            // Transition conditions
            if (filtered_dist < ctx->config.danger_distance) {
                ctx->current_state = STATE_AEB;
            } else if (filtered_dist >= (ctx->config.warning_distance + HYSTERESIS)) {
                ctx->current_state = STATE_CRUISE;
            }
            break;
            
        case STATE_AEB:
            // Engage active electric braking and cut motor power
            FCW_AEB_ApplyBrakes(ctx);
            
            // Keep alerts solid ON
            HAL_GPIO_WritePin(ctx->hw.led_port, ctx->hw.led_pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(ctx->hw.buzzer_port, ctx->hw.buzzer_pin, GPIO_PIN_SET);
            
            // Transition immediately to safety recovery release state
            ctx->current_state = STATE_SAFE_RELEASE;
            break;
            
        case STATE_SAFE_RELEASE:
            // Motor must remain fully stopped & braked for safety
            FCW_AEB_ApplyBrakes(ctx);
            
            // Flashing alerts rapidly to indicate vehicle is locked in safe mode
            if (HAL_GetTick() - ctx->last_warning_toggle >= 100) {
                ctx->last_warning_toggle = HAL_GetTick();
                ctx->warning_toggle_state = !ctx->warning_toggle_state;
                HAL_GPIO_WritePin(ctx->hw.led_port, ctx->hw.led_pin, 
                                  ctx->warning_toggle_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
                HAL_GPIO_WritePin(ctx->hw.buzzer_port, ctx->hw.buzzer_pin, 
                                  ctx->warning_toggle_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            }
            
            // Transition condition: Only release and return to CRUISE when:
            // 1. The obstacle is cleared (Distance >= warning distance + hysteresis)
            // 2. AND the driver fully releases the gas pedal (ADC value <= safe threshold)
            if (filtered_dist >= (ctx->config.warning_distance + HYSTERESIS) && 
                ctx->adc_value <= ctx->config.safe_throttle_limit) {
                
                // Clear alerts
                HAL_GPIO_WritePin(ctx->hw.led_port, ctx->hw.led_pin, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(ctx->hw.buzzer_port, ctx->hw.buzzer_pin, GPIO_PIN_RESET);
                
                // Restore forward drive direction pins
                HAL_GPIO_WritePin(ctx->hw.in1_port, ctx->hw.in1_pin, GPIO_PIN_SET);
                HAL_GPIO_WritePin(ctx->hw.in2_port, ctx->hw.in2_pin, GPIO_PIN_RESET);
                
                ctx->current_state = STATE_CRUISE;
            }
            break;
            
        default:
            ctx->current_state = STATE_CRUISE;
            break;
    }
}

/**
 * @brief Blinks warning LED and Buzzer at a moderate interval in FCW state without blocking.
 * @param ctx Pointer to context
 */
static void FCW_AEB_UpdateAlerts(FcwAebContext_t *ctx) {
    uint32_t current_time = HAL_GetTick();
    const uint32_t BLINK_INTERVAL = 250; // Blink every 250ms
    
    if (current_time - ctx->last_warning_toggle >= BLINK_INTERVAL) {
        ctx->last_warning_toggle = current_time;
        ctx->warning_toggle_state = !ctx->warning_toggle_state;
        
        HAL_GPIO_WritePin(ctx->hw.led_port, ctx->hw.led_pin, 
                          ctx->warning_toggle_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(ctx->hw.buzzer_port, ctx->hw.buzzer_pin, 
                          ctx->warning_toggle_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

/**
 * @brief Adjusts L298N PWM speed (duty cycle 0-100%).
 * @param ctx Pointer to context
 * @param duty Target duty cycle percent (0 to 100)
 */
static void FCW_AEB_SetMotorSpeed(FcwAebContext_t *ctx, uint16_t duty) {
    if (ctx->hw.htim_pwm == NULL) return;
    
    // Ensure duty cycle is clamped in 0% - 100%
    if (duty > 100) duty = 100;
    ctx->motor_pwm_duty = duty;
    
    // Convert percentage to Timer Auto-Reload Register (ARR) value
    // Duty value = (ARR * duty) / 100
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(ctx->hw.htim_pwm);
    uint32_t compare_val = (arr * duty) / 100;
    
    __HAL_TIM_SET_COMPARE(ctx->hw.htim_pwm, ctx->hw.pwm_channel, compare_val);
}

/**
 * @brief Pulls L298N direction control pins to active brake condition and cuts PWM to 0%.
 * @param ctx Pointer to context
 */
static void FCW_AEB_ApplyBrakes(FcwAebContext_t *ctx) {
    // 1. Cut speed to 0% immediately
    FCW_AEB_SetMotorSpeed(ctx, 0);
    
    // 2. Set active electric braking on L298N (IN1 = LOW, IN2 = LOW)
    // Dynamic braking: setting both inputs to the same logic level locks the motor shafts.
    if (ctx->hw.in1_port != NULL && ctx->hw.in2_port != NULL) {
        HAL_GPIO_WritePin(ctx->hw.in1_port, ctx->hw.in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(ctx->hw.in2_port, ctx->hw.in2_pin, GPIO_PIN_RESET);
    }
}
