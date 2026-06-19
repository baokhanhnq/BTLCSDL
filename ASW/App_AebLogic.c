#include "App_AebLogic.h"
#include "Filter_Median.h"
#include "Rte.h"
#include <stddef.h>

static SystemConfig_t s_systemConfig;
static Filter_Median_t s_distanceFilter;

/*
 * Khoi tao logic AEB va bo loc khoang cach.
 */
void App_AebLogic_Init(SystemConfig_t config)
{
    s_systemConfig = config;
    Filter_Median_Init(&s_distanceFilter, 100U);
    Rte_Write_SystemState(STATE_CRUISE);
}

/*
 * Chon nguong canh bao va nguy hiem theo phan tram ga hien tai.
 */
static void App_AebLogic_GetThresholds(uint16_t set_speed,
                                       uint16_t *danger_distance,
                                       uint16_t *warning_distance)
{
    if (set_speed <= THROTTLE_PWM_LEVEL_1_MAX)
    {
        *danger_distance = AEB_DISTANCE_LEVEL_1;
        *warning_distance = FCW_DISTANCE_LEVEL_1;
    }
    else if (set_speed <= THROTTLE_PWM_LEVEL_2_MAX)
    {
        *danger_distance = AEB_DISTANCE_LEVEL_2;
        *warning_distance = FCW_DISTANCE_LEVEL_2;
    }
    else if (set_speed <= THROTTLE_PWM_LEVEL_3_MAX)
    {
        *danger_distance = AEB_DISTANCE_LEVEL_3;
        *warning_distance = FCW_DISTANCE_LEVEL_3;
    }
    else
    {
        *danger_distance = AEB_DISTANCE_LEVEL_4;
        *warning_distance = FCW_DISTANCE_LEVEL_4;
    }
}

/*
 * So do chuyen trang thai AEB/FCW:
 *
 * CRUISE
 *   -> FCW khi vat can di vao vung canh bao.
 *   -> AEB khi vat can di vao vung nguy hiem.
 *
 * FCW
 *   -> AEB khi vat can di vao vung nguy hiem.
 *   -> CRUISE khi vat can da ra xa kem hysteresis.
 *
 * AEB
 *   -> SAFE_RELEASE sau khi kich hoat phanh khan cap.
 *
 * SAFE_RELEASE
 *   -> CRUISE chi khi khoang cach an toan va ga da ha thap.
 */
static SystemState_t App_AebLogic_UpdateState(SystemState_t current_state,
                                              uint16_t filtered_dist,
                                              uint16_t set_speed,
                                              uint16_t danger_distance,
                                              uint16_t warning_distance)
{
    const uint16_t hysteresis = 5U;

    switch (current_state)
    {
        case STATE_CRUISE:
            Rte_Write_BrakeActive(false);

            if ((filtered_dist >= AEB_DISTANCE_MIN) &&
                (filtered_dist <= danger_distance))
            {
                current_state = STATE_AEB;
            }
            else if ((filtered_dist > danger_distance) &&
                     (filtered_dist <= warning_distance))
            {
                current_state = STATE_FCW;
            }
            break;

        case STATE_FCW:
            Rte_Write_BrakeActive(false);

            if ((filtered_dist >= AEB_DISTANCE_MIN) &&
                (filtered_dist <= danger_distance))
            {
                current_state = STATE_AEB;
            }
            else if (filtered_dist >= (warning_distance + hysteresis))
            {
                current_state = STATE_CRUISE;
            }
            break;

        case STATE_AEB:
            Rte_Write_BrakeActive(true);
            current_state = STATE_SAFE_RELEASE;
            break;

        case STATE_SAFE_RELEASE:
            Rte_Write_BrakeActive(true);

            if ((filtered_dist >= (warning_distance + hysteresis)) &&
                (set_speed <= s_systemConfig.safe_throttle_limit))
            {
                Rte_Write_BrakeActive(false);
                current_state = STATE_CRUISE;
            }
            break;

        default:
            current_state = STATE_CRUISE;
            break;
    }

    return current_state;
}

/*
 * Task chinh cua AEB.
 * Doc khoang cach/ga tu RTE, cap nhat trang thai he thong va dieu khien phanh.
 */
void App_AebLogic_Process(void)
{
    uint16_t raw_dist;
    uint16_t filtered_dist;
    uint16_t set_speed;
    uint16_t danger_distance;
    uint16_t warning_distance;
    SystemState_t current_state;

    raw_dist = Rte_Read_RawDistance();
    filtered_dist = Filter_Median_Update(&s_distanceFilter, raw_dist);
    Rte_Write_Distance(filtered_dist);

    set_speed = Rte_Read_ThrottlePercent();
    current_state = Rte_Read_SystemState();
    danger_distance = s_systemConfig.danger_distance;
    warning_distance = s_systemConfig.warning_distance;

    App_AebLogic_GetThresholds(set_speed, &danger_distance, &warning_distance);

    if (set_speed <= MOTOR_STOP_DUTY_MAX)
    {
        Rte_Write_BrakeActive(false);
        Rte_Write_SystemState(STATE_CRUISE);
        return;
    }

    current_state = App_AebLogic_UpdateState(current_state,
                                             filtered_dist,
                                             set_speed,
                                             danger_distance,
                                             warning_distance);

    Rte_Write_SystemState(current_state);
}
