#include "pwm_control.h"

#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_config.h"

static const char *TAG = "pwm_control";

static uint32_t duty_max(void)
{
    return (1U << APP_PWM_RES_BITS) - 1U;
}

esp_err_t pwm_control_init(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = APP_PWM_RES_BITS,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = APP_PWM_FREQ_HZ,
    };

    esp_err_t err = ledc_timer_config(&timer_config);
    if (err != ESP_OK) {
        return err;
    }

    ledc_channel_config_t channel_config = {
        .gpio_num = APP_PWM_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
    };

    err = ledc_channel_config(&channel_config);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "PWM iniciado en GPIO %d, %d Hz, %d bits", APP_PWM_GPIO, APP_PWM_FREQ_HZ, APP_PWM_RES_BITS);
    }
    return err;
}

esp_err_t pwm_control_set_duty_percent(uint8_t duty_percent)
{
    if (duty_percent > 100) {
        duty_percent = 100;
    }

    const uint32_t duty = (duty_max() * duty_percent) / 100U;

    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    if (err != ESP_OK) {
        return err;
    }

    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    return err;
}

void pwm_control_test_sequence(void)
{
    for (uint8_t i = 0; i <= 100; ++i) {
        pwm_control_set_duty_percent(i);
        vTaskDelay(pdMS_TO_TICKS(APP_TEST_DELAY_MS));
    }

    for (uint8_t i = 100; i > 0; --i) {
        pwm_control_set_duty_percent(i);
        vTaskDelay(pdMS_TO_TICKS(APP_TEST_DELAY_MS));
    }
}
