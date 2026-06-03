/**
  ******************************************************************************
  * @file           : fcw_aeb.c
  * @brief          : Forward Collision Warning and Automatic Emergency Braking
  *                   system implementation using modular drivers.
  ******************************************************************************
  */

#include "fcw_aeb.h"
#include "delay.h"
#include "uart.h"
#include <string.h>

/**
 * @brief Initializes the FCW & AEB simulation context and sets up starting states.
 * @param ctx Pointer to the FcwAebContext_t context struct
 * @param motor Configured Motor_Config_t struct
 * @param sensor Configured HCSR04_Config_t struct
 * @param led_buzzer Configured LedBuzzer_Config_t struct
 * @param config System threshold configurations
 */
void FCW_AEB_Init(FcwAebContext_t *ctx, Motor_Config_t motor, HCSR04_Config_t sensor, LedBuzzer_Config_t led_buzzer, SystemConfig_t config) {
    if (ctx == NULL) return;
    
    // Clear context
    memset(ctx, 0, sizeof(FcwAebContext_t));
    
    // Copy configurations
    ctx->motor = motor;
    ctx->sensor = sensor;
    ctx->led_buzzer = led_buzzer;
    ctx->config = config;
    
    // Set default initial values
    ctx->current_state = STATE_CRUISE;
    ctx->raw_distance = 100;
    ctx->filtered_distance = 100;
    
    // Initialize sub-modules
    Delay_DWT_Init();
    Filter_Init(&ctx->filter, 100);
    Motor_Init(&ctx->motor);
    LedBuzzer_Init(&ctx->led_buzzer);
}

/**
 * @brief Runs a single step of the FCW/AEB safety logic, transitions state machine, and controls outputs.
 * @param ctx Pointer to context
 */
void FCW_AEB_Process(FcwAebContext_t *ctx) {
    if (ctx == NULL) return;
    
    // 1. Read throttle value from ADC (simulate accelerator pedal)
    ctx->adc_value = Motor_ReadThrottleAdc(&ctx->motor);
    
    // 2. Convert ADC (0 - 4095) to Throttle Duty (0% - 100%)
    uint16_t target_duty = Motor_AdcToDuty(ctx->adc_value);
    
    // 3. Measure and filter the obstacle distance
    ctx->raw_distance = HCSR04_MeasureDistance(&ctx->sensor);
    ctx->filtered_distance = Filter_Update(&ctx->filter, ctx->raw_distance);
    
    // Define a small hysteresis (e.g. 5cm) to prevent rapid flickering between states
    const uint16_t HYSTERESIS = 5;
    
    // 4. State Machine transitions and action triggers
    switch (ctx->current_state) {
        
        case STATE_CRUISE:
            // Warnings are off in normal mode
            LedBuzzer_SetSolid(&ctx->led_buzzer, false);
            
            // Set motor speed following throttle directly
            Motor_SetSpeed(&ctx->motor, target_duty);
            ctx->motor_pwm_duty = target_duty;
            
            // Transition conditions
            if (ctx->filtered_distance < ctx->config.danger_distance) {
                ctx->current_state = STATE_AEB;
            } else if (ctx->filtered_distance < ctx->config.warning_distance) {
                ctx->current_state = STATE_FCW;
            }
            break;
            
        case STATE_FCW:
            // Drive warning led & buzzer (blink LED & Buzzer moderately at 250ms)
            LedBuzzer_UpdateBlink(&ctx->led_buzzer, 250);
            
            // Safety Option: Limit speed to max 50% in Warning zone
            uint16_t capped_duty = (target_duty > 50) ? 50 : target_duty;
            Motor_SetSpeed(&ctx->motor, capped_duty);
            ctx->motor_pwm_duty = capped_duty;
            
            // Transition conditions
            if (ctx->filtered_distance < ctx->config.danger_distance) {
                ctx->current_state = STATE_AEB;
            } else if (ctx->filtered_distance >= (ctx->config.warning_distance + HYSTERESIS)) {
                ctx->current_state = STATE_CRUISE;
            }
            break;
            
        case STATE_AEB:
            // Engage active electric braking and cut motor power
            Motor_ApplyBrake(&ctx->motor);
            ctx->motor_pwm_duty = 0;
            
            // Keep LED & Buzzer solid ON
            LedBuzzer_SetSolid(&ctx->led_buzzer, true);
            
            // Transition immediately to safety recovery release state
            ctx->current_state = STATE_SAFE_RELEASE;
            break;
            
        case STATE_SAFE_RELEASE:
            // Motor must remain fully stopped & braked for safety
            Motor_ApplyBrake(&ctx->motor);
            ctx->motor_pwm_duty = 0;
            
            // Flashing LED & Buzzer rapidly (100ms) to indicate vehicle is locked in safe mode
            LedBuzzer_UpdateBlink(&ctx->led_buzzer, 100);
            
            // Transition condition: Only release and return to CRUISE when:
            // 1. The obstacle is cleared (Distance >= warning distance + hysteresis)
            // 2. AND the driver fully releases the gas pedal (ADC value <= safe threshold)
            if (ctx->filtered_distance >= (ctx->config.warning_distance + HYSTERESIS) && 
                ctx->adc_value <= ctx->config.safe_throttle_limit) {
                
                // Clear warning LED & Buzzer
                LedBuzzer_SetSolid(&ctx->led_buzzer, false);
                
                // Re-initialize/restore forward drive direction pins
                Motor_Init(&ctx->motor);
                
                ctx->current_state = STATE_CRUISE;
            }
            break;
            
        default:
            ctx->current_state = STATE_CRUISE;
            break;
    }
    
    // 5. Print debug logging to PuTTY via UART
    const char *state_str = "UNKNOWN";
    switch (ctx->current_state) {
        case STATE_CRUISE:       state_str = "CRUISE"; break;
        case STATE_FCW:          state_str = "FCW WARNING"; break;
        case STATE_AEB:          state_str = "AEB BRAKING"; break;
        case STATE_SAFE_RELEASE: state_str = "SAFE RELEASE"; break;
    }
    UART_Printf("State: [%s] | Dist: %d cm (Raw: %d cm) | Throttle: %d%% | ADC: %d\r\n",
                state_str, ctx->filtered_distance, ctx->raw_distance, ctx->motor_pwm_duty, ctx->adc_value);
}
