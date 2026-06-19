#include "Rte.h"
#include "L298N_Driver.h"
#include <stddef.h>

/* Du lieu chia se giua driver BSW va logic ung dung ASW. */
static uint16_t s_rawDistance = 0U;
static uint16_t s_rawDistanceCm10 = 0U;
static uint16_t s_filteredDistance = 0U;
static uint32_t s_throttleAdc = 0U;
static uint16_t s_throttlePercent = 0U;
static uint16_t s_motorSpeed = 0U;
static bool s_brakeActive = false;
static SystemState_t s_systemState = STATE_CRUISE;

/*
 * Luu khoang cach tho theo cm cho logic AEB.
 */
void Rte_Write_RawDistance(uint16_t dist)
{
    s_rawDistance = dist;
}

/*
 * Doc khoang cach tho theo cm.
 */
uint16_t Rte_Read_RawDistance(void)
{
    return s_rawDistance;
}

/*
 * Luu khoang cach tho theo don vi 0.1 cm de log UART.
 */
void Rte_Write_RawDistanceCm10(uint16_t dist_cm10)
{
    s_rawDistanceCm10 = dist_cm10;
}

/*
 * Doc khoang cach tho theo don vi 0.1 cm.
 */
uint16_t Rte_Read_RawDistanceCm10(void)
{
    return s_rawDistanceCm10;
}

/*
 * Luu khoang cach da loc theo cm.
 */
void Rte_Write_Distance(uint16_t dist)
{
    s_filteredDistance = dist;
}

/*
 * Doc khoang cach da loc theo cm.
 */
uint16_t Rte_Read_Distance(void)
{
    return s_filteredDistance;
}

/*
 * Luu gia tri ADC tho cua ga.
 */
void Rte_Write_ThrottleAdc(uint32_t adc)
{
    s_throttleAdc = adc;
}

/*
 * Doc gia tri ADC tho cua ga.
 */
uint32_t Rte_Read_ThrottleAdc(void)
{
    return s_throttleAdc;
}

/*
 * Luu lenh ga theo phan tram.
 */
void Rte_Write_ThrottlePercent(uint16_t percent)
{
    s_throttlePercent = percent;
}

/*
 * Doc lenh ga theo phan tram.
 */
uint16_t Rte_Read_ThrottlePercent(void)
{
    return s_throttlePercent;
}

/*
 * Luu toc do motor va gui den L298N khi phanh khong kich hoat.
 */
void Rte_Write_MotorSpeed(uint16_t speed)
{
    s_motorSpeed = speed;

    if (s_brakeActive == false)
    {
        L298N_SetSpeed(speed);
    }
}

/*
 * Doc toc do motor duoc yeu cau gan nhat.
 */
uint16_t Rte_Read_MotorSpeed(void)
{
    return s_motorSpeed;
}

/*
 * Bat hoac nha phanh. Khoi tao lai L298N khi nha phanh.
 */
void Rte_Write_BrakeActive(bool active)
{
    if (active == s_brakeActive)
    {
        return;
    }

    s_brakeActive = active;
    if (active)
    {
        L298N_ApplyBrake();
        s_motorSpeed = 0U;
    }
    else
    {
        L298N_Init();
    }
}

/*
 * Doc trang thai phanh hien tai.
 */
bool Rte_Read_BrakeActive(void)
{
    return s_brakeActive;
}

/*
 * Luu trang thai he thong hien tai.
 */
void Rte_Write_SystemState(SystemState_t state)
{
    s_systemState = state;
}

/*
 * Doc trang thai he thong hien tai.
 */
SystemState_t Rte_Read_SystemState(void)
{
    return s_systemState;
}
