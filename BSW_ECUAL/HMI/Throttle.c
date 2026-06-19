#include "Throttle.h"
#include "adc.h"
#include "Rte.h"
#include "system_config.h"

/*
 * Khoi tao ngo vao ADC cua bien tro ga.
 */
void Throttle_Init(void)
{
    ADC1_Init();
}

/*
 * Doc gia tri ADC 12-bit tu bien tro ga.
 */
uint32_t Throttle_ReadAdc(void)
{
    return (uint32_t)ADC1_Read();
}

/*
 * Chuyen gia tri ADC tho sang phan tram ga.
 */
uint16_t Throttle_GetPercent(uint32_t adc_val)
{
    uint32_t duty;

    duty = (adc_val * 100U) / 4095U;
    if (duty > 100U)
    {
        duty = 100U;
    }

    return (uint16_t)duty;
}

/*
 * Task doc tin hieu ga.
 * Doc ADC va luu gia tri tho/phan tram vao RTE.
 */
void Throttle_Process(void)
{
    uint32_t adc;
    uint16_t percent;

    adc = Throttle_ReadAdc();
    percent = Throttle_GetPercent(adc);

    Rte_Write_ThrottleAdc(adc);
    Rte_Write_ThrottlePercent(percent);
}

/*
 * Task thuc thi dieu khien ga.
 * Doc trang thai AEB tu RTE va gui duty motor duoc phep den L298N qua RTE.
 */
void Throttle_Execute(void)
{
    SystemState_t state;
    uint16_t target_duty;
    uint32_t throttle_adc;

    state = Rte_Read_SystemState();
    target_duty = Rte_Read_ThrottlePercent();
    throttle_adc = Rte_Read_ThrottleAdc();

    if ((throttle_adc == 0U) || (target_duty <= MOTOR_STOP_DUTY_MAX))
    {
        target_duty = 0U;
    }

    if (state == STATE_CRUISE)
    {
        Rte_Write_MotorSpeed(target_duty);
    }
    else if (state == STATE_FCW)
    {
        uint16_t capped_duty;

        capped_duty = (target_duty > FCW_MAX_DUTY) ? FCW_MAX_DUTY : target_duty;
        Rte_Write_MotorSpeed(capped_duty);
    }
    else
    {
        /* AEB va Safe Release ep toc do motor ve 0. */
        Rte_Write_MotorSpeed(0U);
    }
}
