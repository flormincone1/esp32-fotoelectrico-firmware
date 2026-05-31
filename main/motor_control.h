#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

typedef enum {
    MOTOR_DIR_FORWARD = 0,
    MOTOR_DIR_REVERSE,
} motor_direction_t;

esp_err_t motor_control_init(void);
esp_err_t motor_control_set_direction(motor_direction_t direction);
esp_err_t motor_control_set_speed_percent(uint8_t speed_percent);
esp_err_t motor_control_stop(void);
void motor_control_test_sequence(void);
