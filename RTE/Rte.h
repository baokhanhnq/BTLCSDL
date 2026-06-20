#ifndef RTE_RTE_H_
#define RTE_RTE_H_

#include <stdint.h>
#include <stdbool.h>
#include "system_config.h"

/* API du lieu cam bien */
void Rte_Write_RawDistance(uint16_t dist);
uint16_t Rte_Read_RawDistance(void);

void Rte_Write_RawDistanceCm10(uint16_t dist_cm10);
uint16_t Rte_Read_RawDistanceCm10(void);

void Rte_Write_FilterDistance(uint16_t dist);
uint16_t Rte_Read_FilterDistance(void);

/* API ga nguoi dung */
void Rte_Write_ThrottleAdc(uint32_t adc);
uint32_t Rte_Read_ThrottleAdc(void);

void Rte_Write_ThrottlePercent(uint16_t percent);
uint16_t Rte_Read_ThrottlePercent(void);

/* API co cau chap hanh motor */
void Rte_Write_MotorSpeed(uint16_t speed);
uint16_t Rte_Read_MotorSpeed(void);
void Rte_Write_BrakeActive(bool active);
bool Rte_Read_BrakeActive(void);

/* API trang thai he thong */
void Rte_Write_SystemState(SystemState_t state);
SystemState_t Rte_Read_SystemState(void);

#endif /* RTE_RTE_H_ */
