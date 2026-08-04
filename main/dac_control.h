#pragma once

#include <stdint.h>

#include "esp_err.h"

esp_err_t dac_control_init(void);
esp_err_t dac_control_set_raw(uint8_t value);
esp_err_t dac_control_set_percent(uint8_t percent);
void dac_control_test_sequence(void);
