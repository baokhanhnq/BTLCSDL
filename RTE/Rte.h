#ifndef RTE_RTE_H_
#define RTE_RTE_H_

#include <stdint.h>
#include <stdbool.h>
#include "system_config.h"

/* RTE Initialization */
void Rte_Init(void);

/* Sensor Data APIs */
void Rte_Write_RawDistance(uint16_t dist);
uint16_t Rte_Read_RawDistance(void);

void Rte_Write_RawDistanceCm10(uint16_t dist_cm10);
uint16_t Rte_Read_RawDistanceCm10(void);

void Rte_Write_Distance(uint16_t dist);
uint16_t Rte_Read_Distance(void);

/* HMI Throttle APIs */
void Rte_Write_ThrottleAdc(uint32_t adc);
uint32_t Rte_Read_ThrottleAdc(void);

void Rte_Write_ThrottlePercent(uint16_t percent);
uint16_t Rte_Read_ThrottlePercent(void);

/* Motor Actuator APIs */
void Rte_Write_MotorSpeed(uint16_t speed);
uint16_t Rte_Read_MotorSpeed(void);
void Rte_Write_BrakeActive(bool active);
bool Rte_Read_BrakeActive(void);

/* System State APIs */
void Rte_Write_SystemState(SystemState_t state);
SystemState_t Rte_Read_SystemState(void);

/* Indicator & Alert APIs */
void Rte_Write_Alerts(SystemState_t state);

/* System Status Logger API */
void Rte_LogStatus(void);

#endif /* RTE_RTE_H_ */
