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
    Rte_Write_BrakeActive(false);
}

void App_AebLogic_Process(void)
{
    uint16_t filtered_dist;
    uint16_t set_speed;
    uint16_t safe_throttle_limit;
    uint16_t danger_dist;
    uint16_t warning_dist;
    SystemState_t state;
    SystemState_t new_state;

    filtered_dist = Filter_Median_Update(&s_distanceFilter,
                                         Rte_Read_RawDistance());
    /* Luu khoang cach da loc rieng de UART log va de debug de doc hon. */
    Rte_Write_FilterDistance(filtered_dist);

    set_speed = Rte_Read_ThrottlePercent();
    safe_throttle_limit = s_systemConfig.safe_throttle_limit;
    state = Rte_Read_SystemState();
    new_state = state;

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

    /* Khi chua bi AEB giu lai, ga qua nho thi coi nhu xe dung va ve CRUISE. */
    if ((set_speed <= MOTOR_STOP_DUTY_MAX) &&
        (state != STATE_AEB) &&
        (state != STATE_SAFE_RELEASE))
    {
        /* Khong kich hoat FCW/AEB khi nguoi lai khong yeu cau xe chay. */
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

                /* Giu AEB den khi thoat vung nguy hiem. */
                if (filtered_dist > danger_dist)
                {
                    if ((filtered_dist >=
                         (warning_dist + DISTANCE_HYSTERESIS)) &&
                        (set_speed <= safe_throttle_limit))
                    {
                        /* Chi ve CRUISE khi vat da an toan va tay ga da ha xuong. */
                        new_state = STATE_CRUISE;
                    }
                    else
                    {
                        /* Neu moi chi thoat AEB nhung chua du dieu kien nha, tiep tuc giu phanh. */
                        new_state = STATE_SAFE_RELEASE;
                    }
                }
                break;

            case STATE_SAFE_RELEASE:

                if ((filtered_dist >= AEB_DISTANCE_MIN) &&
                         (filtered_dist <= danger_dist))
                {
                    /* Vat can quay lai gan trong luc dang nha phanh thi vao AEB lai. */
                    new_state = STATE_AEB;
                }
                else if ((filtered_dist >=
                          (warning_dist + DISTANCE_HYSTERESIS)) &&
                         (set_speed <= safe_throttle_limit))
                {
                    /* Du ca hai dieu kien: khoang cach an toan va ga <= nguong nha phanh. */
                    new_state = STATE_CRUISE;
                }
                else
                {
                    /* Con thieu mot trong hai dieu kien nen tiep tuc giu SAFE_RELEASE. */
                    new_state = STATE_SAFE_RELEASE;
                }
                break;

            default:
                new_state = STATE_CRUISE;
                break;
        }
    }

    /* Cap nhat trang thai he thong */
    Rte_Write_SystemState(new_state);
    Rte_Write_BrakeActive((new_state == STATE_AEB) ||
                          (new_state == STATE_SAFE_RELEASE));
}
