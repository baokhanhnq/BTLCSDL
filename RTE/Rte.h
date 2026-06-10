#ifndef RTE_RTE_H_
#define RTE_RTE_H_

#include <stdint.h>
#include <stdbool.h>
#include "system_config.h"

/* RTE Initialization and Main Processing */
void Rte_Init(void);
void Rte_Update(void);

/* Sensor Data APIs */
uint16_t Rte_Read_RawDistance(void);
void Rte_Write_FilteredDistance(uint16_t dist);
uint16_t Rte_Read_FilteredDistance(void);

/* HMI Throttle APIs */
uint32_t Rte_Read_ThrottleAdc(void);
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
void Rte_Write_Alerts(bool green, bool yellow, bool red, bool buzzer);
void Rte_Update_AlertsBlink(bool green, bool yellow, bool red, bool buzzer, uint32_t interval_ms);

/* System Status Logger API */
void Rte_LogStatus(void);

#endif /* RTE_RTE_H_ */
