#include "App_CruiseCtrl.h"
#include "Rte.h"
#include "system_config.h"

void App_CruiseCtrl_Init(void) {
    /* No state variable initialisation needed for basic Cruise Control */
}

void App_CruiseCtrl_Process(void) {
    SystemState_t state = Rte_Read_SystemState();
    /* Đọc ADC trước để RTE cập nhật lại throttlePercent mới nhất */
    uint32_t throttle_adc = Rte_Read_ThrottleAdc();
    uint16_t target_duty = Rte_Read_ThrottlePercent();

    if (throttle_adc == 0U || target_duty <= MOTOR_STOP_DUTY_MAX) {
        target_duty = 0U;
    }
    
    if (state == STATE_CRUISE) {
        Rte_Write_MotorSpeed(target_duty);
    } else if (state == STATE_FCW) {
        uint16_t capped_duty = (target_duty > FCW_MAX_DUTY) ? FCW_MAX_DUTY : target_duty;
        Rte_Write_MotorSpeed(capped_duty);
    } else {
        /* Under AEB or Safe Release, speed is forced to 0.
           Braking is handled by AEB logic via Rte_Write_BrakeActive(true) */
        Rte_Write_MotorSpeed(0);
    }
}
