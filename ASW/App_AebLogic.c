#include "App_AebLogic.h"
#include "Filter_Median.h"
#include "Rte.h"
#include <stddef.h>

#define DISTANCE_HYSTERESIS    (5U)

static SystemConfig_t  s_systemConfig;
static Filter_Median_t s_distanceFilter;

void App_AebLogic_Init(SystemConfig_t config)
{
    s_systemConfig = config;

    /* Khoi tao bo loc khoang cach */
    Filter_Median_Init(&s_distanceFilter, 100U);

    /* Trang thai mac dinh */
    Rte_Write_SystemState(STATE_CRUISE);
}

void App_AebLogic_Process(void)
{
    uint16_t filtered_dist =
        Filter_Median_Update(&s_distanceFilter,
                             Rte_Read_RawDistance());

    uint16_t set_speed = Rte_Read_ThrottlePercent();
#include "App_AebLogic.h"
#include "Filter_Median.h"
#include "Rte.h"
#include <stddef.h>

#define DISTANCE_HYSTERESIS    (5U)

static SystemConfig_t  s_systemConfig;
static Filter_Median_t s_distanceFilter;

void App_AebLogic_Init(SystemConfig_t config)
{
    s_systemConfig = config;

    /* Khoi tao bo loc khoang cach */
    Filter_Median_Init(&s_distanceFilter, 100U);

    /* Trang thai mac dinh */
    Rte_Write_SystemState(STATE_CRUISE);
}

void App_AebLogic_Process(void)
{
    uint16_t filtered_dist =
        Filter_Median_Update(&s_distanceFilter,
                             Rte_Read_RawDistance());

    uint16_t set_speed = Rte_Read_ThrottlePercent();

    SystemState_t state     = Rte_Read_SystemState();
    SystemState_t new_state = state;

    uint16_t danger_dist;
    uint16_t warning_dist;

    /* Chon nguong theo toc do */
    if (set_speed <= THROTTLE_PWM_LEVEL_1_MAX)
    {
        danger_dist  = AEB_DISTANCE_LEVEL_1;
        warning_dist = FCW_DISTANCE_LEVEL_1;
    }
    else if (set_speed <= THROTTLE_PWM_LEVEL_2_MAX)
    {
        danger_dist  = AEB_DISTANCE_LEVEL_2;
        warning_dist = FCW_DISTANCE_LEVEL_2;
    }
    else if (set_speed <= THROTTLE_PWM_LEVEL_3_MAX)
    {
        danger_dist  = AEB_DISTANCE_LEVEL_3;
        warning_dist = FCW_DISTANCE_LEVEL_3;
    }
    else
    {
        danger_dist  = AEB_DISTANCE_LEVEL_4;
        warning_dist = FCW_DISTANCE_LEVEL_4;
    }

    /* Xe dang dung */
    if (set_speed <= MOTOR_STOP_DUTY_MAX)
    {
        new_state = STATE_CRUISE;
    }
    else
    {
        switch (state)
        {
            case STATE_CRUISE:

                /* Chuyen sang FCW hoac AEB */
                if ((filtered_dist >= AEB_DISTANCE_MIN) &&
                    (filtered_dist <= danger_dist))
                {
                    new_state = STATE_AEB;
                }
                else if ((filtered_dist > danger_dist) &&
                         (filtered_dist <= warning_dist))
                {
                    new_state = STATE_FCW;
                }
                break;

            case STATE_FCW:

                /* Chuyen sang AEB hoac quay lai CRUISE */
                if ((filtered_dist >= AEB_DISTANCE_MIN) &&
                    (filtered_dist <= danger_dist))
                {
                    new_state = STATE_AEB;
                }
                else if (filtered_dist >=
                         (warning_dist + DISTANCE_HYSTERESIS))
                {
                    new_state = STATE_CRUISE;
                }
                break;

            case STATE_AEB:

                /* Giu AEB den khi thoat vung nguy hiem */
                if (filtered_dist > danger_dist)
                {
                    new_state = STATE_SAFE_RELEASE;
                }
                break;

            case STATE_SAFE_RELEASE:

                /* Quay lai CRUISE khi da an toan */
                if (filtered_dist >=
                    (warning_dist + DISTANCE_HYSTERESIS))
                {
                    new_state = STATE_CRUISE;
                }
                /* Vat can xuat hien lai */
                else if ((filtered_dist >= AEB_DISTANCE_MIN) &&
                         (filtered_dist <= danger_dist))
                {
                    new_state = STATE_AEB;
                }
                break;

            default:
                new_state = STATE_CRUISE;
                break;
        }
    }

    /* Cap nhat trang thai he thong */
    Rte_Write_SystemState(new_state);
}
    SystemState_t state     = Rte_Read_SystemState();
    SystemState_t new_state = state;

    uint16_t danger_dist;
    uint16_t warning_dist;

    /* Chon nguong theo toc do */
    if (set_speed <= THROTTLE_PWM_LEVEL_1_MAX)
    {
        danger_dist  = AEB_DISTANCE_LEVEL_1;
        warning_dist = FCW_DISTANCE_LEVEL_1;
    }
    else if (set_speed <= THROTTLE_PWM_LEVEL_2_MAX)
    {
        danger_dist  = AEB_DISTANCE_LEVEL_2;
        warning_dist = FCW_DISTANCE_LEVEL_2;
    }
    else if (set_speed <= THROTTLE_PWM_LEVEL_3_MAX)
    {
        danger_dist  = AEB_DISTANCE_LEVEL_3;
        warning_dist = FCW_DISTANCE_LEVEL_3;
    }
    else
    {
        danger_dist  = AEB_DISTANCE_LEVEL_4;
        warning_dist = FCW_DISTANCE_LEVEL_4;
    }

    /* Xe dang dung */
    if (set_speed <= MOTOR_STOP_DUTY_MAX)
    {
        new_state = STATE_CRUISE;
    }
    else
    {
        switch (state)
        {
            case STATE_CRUISE:

                /* Chuyen sang FCW hoac AEB */
                if ((filtered_dist >= AEB_DISTANCE_MIN) &&
                    (filtered_dist <= danger_dist))
                {
                    new_state = STATE_AEB;
                }
                else if ((filtered_dist > danger_dist) &&
                         (filtered_dist <= warning_dist))
                {
                    new_state = STATE_FCW;
                }
                break;

            case STATE_FCW:

                /* Chuyen sang AEB hoac quay lai CRUISE */
                if ((filtered_dist >= AEB_DISTANCE_MIN) &&
                    (filtered_dist <= danger_dist))
                {
                    new_state = STATE_AEB;
                }
                else if (filtered_dist >=
                         (warning_dist + DISTANCE_HYSTERESIS))
                {
                    new_state = STATE_CRUISE;
                }
                break;

            case STATE_AEB:

                /* Giu AEB den khi thoat vung nguy hiem */
                if (filtered_dist > danger_dist)
                {
                    new_state = STATE_SAFE_RELEASE;
                }
                break;

            case STATE_SAFE_RELEASE:

                /* Quay lai CRUISE khi da an toan */
                if (filtered_dist >=
                    (warning_dist + DISTANCE_HYSTERESIS))
                {
                    new_state = STATE_CRUISE;
                }
                /* Vat can xuat hien lai */
                else if ((filtered_dist >= AEB_DISTANCE_MIN) &&
                         (filtered_dist <= danger_dist))
                {
                    new_state = STATE_AEB;
                }
                break;

            default:
                new_state = STATE_CRUISE;
                break;
        }
    }

    /* Cap nhat trang thai he thong */
    Rte_Write_SystemState(new_state);
}