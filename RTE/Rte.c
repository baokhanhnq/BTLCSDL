#include "Rte.h"
#include <stddef.h>

static uint16_t rawDistance = 0;
static uint16_t filteredDistance = 0;
static uint32_t throttleAdc = 0;
static uint16_t throttlePercent = 0;
static uint16_t motorSpeed = 0;
static bool brakeActive = false;
static SystemState_t systemState = STATE_CRUISE;

uint16_t Rte_Read_RawDistance(void) {
    return rawDistance;
}

void Rte_Write_RawDistance(uint16_t dist) {
    rawDistance = dist;
}

void Rte_Write_FilteredDistance(uint16_t dist) {
    filteredDistance = dist;
}

uint16_t Rte_Read_FilteredDistance(void) {
    return filteredDistance;
}

uint32_t Rte_Read_ThrottleAdc(void) {
    return throttleAdc;
}

void Rte_Write_ThrottleAdc(uint32_t adc) {
    throttleAdc = adc;
}

uint16_t Rte_Read_ThrottlePercent(void) {
    return throttlePercent;
}

void Rte_Write_ThrottlePercent(uint16_t percent) {
    throttlePercent = percent;
}

void Rte_Write_MotorSpeed(uint16_t speed) {
    motorSpeed = speed;
}

uint16_t Rte_Read_MotorSpeed(void) {
    return motorSpeed;
}

void Rte_Write_BrakeActive(bool active) {
    brakeActive = active;
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