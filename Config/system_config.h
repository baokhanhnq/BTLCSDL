#ifndef CONFIG_SYSTEM_CONFIG_H_
#define CONFIG_SYSTEM_CONFIG_H_

#include <stdint.h>

/* Cac trang thai cua he thong */
typedef enum {
    STATE_CRUISE = 0,
    STATE_FCW,
    STATE_AEB,
    STATE_SAFE_RELEASE
} SystemState_t;

/* Cau truc cau hinh he thong */
typedef struct {
    uint16_t warning_distance;
    uint16_t danger_distance;
    uint16_t safe_throttle_limit;
} SystemConfig_t;

/* Cau hinh mac dinh */
#define DEFAULT_WARNING_DISTANCE     50   /* Canh bao khi khoang cach < 50 cm */
#define DEFAULT_DANGER_DISTANCE      20   /* Phanh khi khoang cach < 20 cm */
#define DEFAULT_SAFE_THROTTLE_LIMIT  10U  /* Duoc nha phanh khi ga <= 10% */

/* Dac tinh motor/PWM da do */
#define MOTOR_STOP_DUTY_MAX          39U  /* 0-39%: coi nhu banh xe dung */
#define MOTOR_MIN_MOVING_DUTY        40U  /* Banh xe bat dau quay khoang 40% PWM */
#define FCW_MAX_DUTY                 50U

/* Nguong khoang cach theo vung duty/ga */
#define THROTTLE_PWM_LEVEL_1_MAX     55U
#define THROTTLE_PWM_LEVEL_2_MAX     70U
#define THROTTLE_PWM_LEVEL_3_MAX     85U
#define THROTTLE_PWM_LEVEL_4_MAX     100U

#define AEB_DISTANCE_MIN             10U

#define AEB_DISTANCE_LEVEL_1         20U
#define AEB_DISTANCE_LEVEL_2         25U
#define AEB_DISTANCE_LEVEL_3         35U
#define AEB_DISTANCE_LEVEL_4         45U

#define FCW_DISTANCE_LEVEL_1         40U
#define FCW_DISTANCE_LEVEL_2         55U
#define FCW_DISTANCE_LEVEL_3         65U
#define FCW_DISTANCE_LEVEL_4         75U

#endif /* CONFIG_SYSTEM_CONFIG_H_ */
