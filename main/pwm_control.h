#pragma once

#include <stdint.h>
#include "esp_err.h"

esp_err_t pwm_control_init(void);
esp_err_t pwm_control_set_duty_percent(uint8_t duty_percent);
void pwm_control_test_sequence(void);
