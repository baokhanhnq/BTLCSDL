#include "Rte.h"
#include "HCSR04.h"
#include "L298N_Driver.h"
#include "Throttle.h"
#include "Alerts.h"
#include "delay.h"
#include "uart.h"
#include <stddef.h>

/* Internal RTE State Variables */
static uint16_t rawDistance = 0;
static uint16_t rawDistanceCm10 = 0;
static uint16_t filteredDistance = 0;
static uint32_t throttleAdc = 0;
static uint16_t throttlePercent = 0;
static uint16_t motorSpeed = 0;
static bool brakeActive = false;
static SystemState_t systemState = STATE_CRUISE;

void Rte_Init(void) {
    delay_init();
    HCSR04_Init();
    L298N_Init();
    Throttle_Init();
    Alerts_Init();
}

void Rte_Write_RawDistance(uint16_t dist) {
    rawDistance = dist;
}

uint16_t Rte_Read_RawDistance(void) {
    return rawDistance;
}

void Rte_Write_RawDistanceCm10(uint16_t dist_cm10) {
    rawDistanceCm10 = dist_cm10;
}

uint16_t Rte_Read_RawDistanceCm10(void) {
    return rawDistanceCm10;
}

void Rte_Write_Distance(uint16_t dist) {
    filteredDistance = dist;
}

uint16_t Rte_Read_Distance(void) {
    return filteredDistance;
}

void Rte_Write_ThrottleAdc(uint32_t adc) {
    throttleAdc = adc;
}

uint32_t Rte_Read_ThrottleAdc(void) {
    return throttleAdc;
}

void Rte_Write_ThrottlePercent(uint16_t percent) {
    throttlePercent = percent;
}

uint16_t Rte_Read_ThrottlePercent(void) {
    return throttlePercent;
}

void Rte_Write_MotorSpeed(uint16_t speed) {
    motorSpeed = speed;
    if (!brakeActive) {
        L298N_SetSpeed(speed);
    }
}

uint16_t Rte_Read_MotorSpeed(void) {
    return motorSpeed;
}

void Rte_Write_BrakeActive(bool active) {
    if (active == brakeActive) {
        return;
    }

    brakeActive = active;
    if (active) {
        L298N_ApplyBrake();
        motorSpeed = 0;
    } else {
        /* Reinitialize motor control pins for forward drive */
        L298N_Init();
    }
}

bool Rte_Read_BrakeActive(void) {
    return brakeActive;
}

void Rte_Write_SystemState(SystemState_t state) {
    systemState = state;
}

SystemState_t Rte_Read_SystemState(void) {
    return systemState;
}

void Rte_Write_Alerts(SystemState_t state) {
    switch (state) {
        case STATE_CRUISE:
            Alerts_SetSolid(true, false, false, false);
            break;
        case STATE_FCW:
            Alerts_UpdateBlink(false, true, false, true, 250U);
            break;
        case STATE_AEB:
            Alerts_SetSolid(false, false, true, true);
            break;
        case STATE_SAFE_RELEASE:
            Alerts_UpdateBlink(false, false, true, true, 100U);
            break;
        default:
            Alerts_SetSolid(false, false, false, false);
            break;
    }
}

void Rte_LogStatus(void) {
    const char *state_str = "UNKNOWN";
    switch (systemState) {
        case STATE_CRUISE:       state_str = "CRUISE"; break;
        case STATE_FCW:          state_str = "FCW WARNING"; break;
        case STATE_AEB:          state_str = "AEB BRAKING"; break;
        case STATE_SAFE_RELEASE: state_str = "SAFE RELEASE"; break;
    }
    UART_Printf("State: [%s] | Dist: %d cm (Raw: %d.%d cm) | Throttle: %d%% | Motor: %d%% | ADC: %d\r\n",
                state_str,
                filteredDistance,
                rawDistanceCm10 / 10U,
                rawDistanceCm10 % 10U,
                throttlePercent,
                motorSpeed,
                throttleAdc);
}
