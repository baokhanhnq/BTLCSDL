#ifndef RTE_H
#define RTE_H

#include <stdint.h>
#include <stdbool.h>

/* Các trạng thái của Hệ thống (Giữ nguyên cho ASW dùng) */
typedef enum {
    STATE_CRUISE,
    STATE_FCW,
    STATE_AEB,
    STATE_SAFE_RELEASE
} SystemState_t;

/* --- API TRUNG CHUYỂN DỮ LIỆU CỦA RTE --- */

/* Cảm biến khoảng cách (Distance) */
uint16_t Rte_Read_RawDistance(void);
void Rte_Write_RawDistance(uint16_t dist);

uint16_t Rte_Read_FilteredDistance(void);
void Rte_Write_FilteredDistance(uint16_t dist);

/* Cảm biến tay ga (Throttle) */
uint32_t Rte_Read_ThrottleAdc(void);
void Rte_Write_ThrottleAdc(uint32_t adc);

uint16_t Rte_Read_ThrottlePercent(void);
void Rte_Write_ThrottlePercent(uint16_t percent);

/* Động cơ & Phanh (Motor & Brake) */
uint16_t Rte_Read_MotorSpeed(void);
void Rte_Write_MotorSpeed(uint16_t speed);

bool Rte_Read_BrakeActive(void);
void Rte_Write_BrakeActive(bool active);

/* Trạng thái hệ thống (System State) */
SystemState_t Rte_Read_SystemState(void);
void Rte_Write_SystemState(SystemState_t state);

#endif /* RTE_H */