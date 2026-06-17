#include "App_AebLogic.h"
#include "Filter_Median.h"
#include "Rte.h"
#include <stddef.h>

static SystemConfig_t systemConfig;
static Filter_Median_t distanceFilter;

static void App_AebLogic_GetThresholds(uint16_t set_speed,
                                       uint16_t *danger_distance,
                                       uint16_t *warning_distance) {
    if (set_speed <= THROTTLE_PWM_LEVEL_1_MAX) {
        *danger_distance = AEB_DISTANCE_LEVEL_1;
        *warning_distance = FCW_DISTANCE_LEVEL_1;
    } else if (set_speed <= THROTTLE_PWM_LEVEL_2_MAX) {
        *danger_distance = AEB_DISTANCE_LEVEL_2;
        *warning_distance = FCW_DISTANCE_LEVEL_2;
    } else if (set_speed <= THROTTLE_PWM_LEVEL_3_MAX) {
        *danger_distance = AEB_DISTANCE_LEVEL_3;
        *warning_distance = FCW_DISTANCE_LEVEL_3;
    } else {
        *danger_distance = AEB_DISTANCE_LEVEL_4;
        *warning_distance = FCW_DISTANCE_LEVEL_4;
    }
}

void App_AebLogic_Init(SystemConfig_t config) {
    systemConfig = config;
    Filter_Median_Init(&distanceFilter, 100);
    Rte_Write_SystemState(STATE_CRUISE);
}

void App_AebLogic_Process(void) {
    uint16_t raw_dist = Rte_Read_RawDistance();
    uint16_t filtered_dist = Filter_Median_Update(&distanceFilter, raw_dist);
    Rte_Write_Distance(filtered_dist);

    uint16_t set_speed = Rte_Read_ThrottlePercent();
    SystemState_t current_state = Rte_Read_SystemState();
    uint16_t danger_distance = systemConfig.danger_distance;
    uint16_t warning_distance = systemConfig.warning_distance;
    
    const uint16_t HYSTERESIS = 5;
    App_AebLogic_GetThresholds(set_speed, &danger_distance, &warning_distance);

    if (set_speed <= MOTOR_STOP_DUTY_MAX) {
        Rte_Write_BrakeActive(false);
        Rte_Write_Alerts(STATE_CRUISE);
        Rte_Write_SystemState(STATE_CRUISE);
        Rte_LogStatus();
        return;
    }
    
    switch (current_state) {
        case STATE_CRUISE:
            Rte_Write_BrakeActive(false);
            Rte_Write_Alerts(STATE_CRUISE);
            
            if (filtered_dist >= AEB_DISTANCE_MIN &&
                filtered_dist <= danger_distance) {
                current_state = STATE_AEB;
            } else if (filtered_dist > danger_distance &&
                       filtered_dist <= warning_distance) {
                current_state = STATE_FCW;
            }
            break;
            
        case STATE_FCW:
            Rte_Write_BrakeActive(false);
            Rte_Write_Alerts(STATE_FCW);
            
            if (filtered_dist >= AEB_DISTANCE_MIN &&
                filtered_dist <= danger_distance) {
                current_state = STATE_AEB;
            } else if (filtered_dist >= (warning_distance + HYSTERESIS)) {
                current_state = STATE_CRUISE;
            }
            break;
            
        case STATE_AEB:
            Rte_Write_BrakeActive(true);
            Rte_Write_Alerts(STATE_AEB);
            current_state = STATE_SAFE_RELEASE;
            break;
            
        case STATE_SAFE_RELEASE:
            Rte_Write_BrakeActive(true);
            Rte_Write_Alerts(STATE_SAFE_RELEASE);
            
            if (filtered_dist >= (warning_distance + HYSTERESIS) &&
                set_speed <= systemConfig.safe_throttle_limit) {
                
                Rte_Write_Alerts(STATE_CRUISE);
                Rte_Write_BrakeActive(false);
                current_state = STATE_CRUISE;
            }
            break;
            
        default:
            current_state = STATE_CRUISE;
            break;
    }
    
    Rte_Write_SystemState(current_state);
    
    /* Log system status to UART */
    Rte_LogStatus();
}
