#include "Throttle.h"
#include "adc.h"
#include "Rte.h"
#include "system_config.h"
#include "L298N_Driver.h"

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
static uint32_t Throttle_ReadAdc(void)
{
    return (uint32_t)ADC1_Read();
}

/*
 * Chuyen gia tri ADC tho sang phan tram ga.
 */
static uint16_t Throttle_GetPercent(uint32_t adc_val)
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
 * Doc state/lenh phanh tu RTE va gui duty motor duoc phep den L298N.
 */
void Throttle_Execute(void)
{
    SystemState_t state;
    uint16_t target_duty;
    uint16_t command_duty;
    uint32_t throttle_adc;
    bool brake_active;

    state = Rte_Read_SystemState();
    target_duty = Rte_Read_ThrottlePercent();
    throttle_adc = Rte_Read_ThrottleAdc();
    brake_active = Rte_Read_BrakeActive();

    /* ADC bang 0 hoac duty nho hon nguong motor dung thi ep toc do ve 0. */
    if ((throttle_adc == 0U) || (target_duty <= MOTOR_STOP_DUTY_MAX))
    {
        Rte_Write_MotorSpeed(0U);
        if (brake_active != false)
        {
            /* Neu AEB dang yeu cau phanh thi tiep tuc giu brake. */
            L298N_ApplyBrake();
        }
        else
        {
            /* Neu khong phanh thi chi tat PWM ve 0%. */
            L298N_SetSpeed(0U);
        }
    }
    /* AEB va SAFE_RELEASE luon uu tien hon lenh ga cua nguoi lai. */
    else if ((brake_active != false) ||
             (state == STATE_AEB) ||
             (state == STATE_SAFE_RELEASE))
    {
        Rte_Write_MotorSpeed(0U);
        L298N_ApplyBrake();
    }
    else
    {
        /* CRUISE dung duty theo ga, FCW gioi han toc do toi da. */
        if (state == STATE_CRUISE)
        {
            command_duty = target_duty;
        }
        else if (state == STATE_FCW)
        {
            command_duty = (target_duty > FCW_MAX_DUTY) ? FCW_MAX_DUTY : target_duty;
        }
        else
        {
            command_duty = 0U;
        }

        Rte_Write_MotorSpeed(command_duty);
        L298N_SetSpeed(command_duty);
    }
}
