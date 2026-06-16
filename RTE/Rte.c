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

uint16_t Rte_Read_RawDistance(void) {
    /* Đọc khoảng cách mới khi ASW yêu cầu, không phụ thuộc Rte_Update() */
    rawDistanceCm10 = (uint16_t)HCSR04_ReadCm10();
    rawDistance = (rawDistanceCm10 + 5U) / 10U;
    return rawDistance;
}

void Rte_Write_FilteredDistance(uint16_t dist) {
    filteredDistance = dist;
}

uint16_t Rte_Read_FilteredDistance(void) {
    return filteredDistance;
}

uint32_t Rte_Read_ThrottleAdc(void) {
    /* Cập nhật cả ADC và phần trăm ga từ giá trị throttle mới nhất */
    throttleAdc = Throttle_ReadAdc();
    throttlePercent = Throttle_GetPercent(throttleAdc);
    return throttleAdc;
}

uint16_t Rte_Read_ThrottlePercent(void) {
    /* Đảm bảo AEB/Cruise không đọc lại throttlePercent cũ */
    throttleAdc = Throttle_ReadAdc();
    throttlePercent = Throttle_GetPercent(throttleAdc);
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

void Rte_Write_Alerts(bool green, bool yellow, bool red, bool buzzer) {
    Alerts_SetSolid(green, yellow, red, buzzer);
}

void Rte_Update_AlertsBlink(bool green, bool yellow, bool red, bool buzzer, uint32_t interval_ms) {
    Alerts_UpdateBlink(green, yellow, red, buzzer, interval_ms);
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
