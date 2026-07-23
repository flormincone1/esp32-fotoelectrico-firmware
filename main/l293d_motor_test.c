#include "l293d_motor_test.h"

#include <stdint.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_config.h"

static const char *TAG = "l293d_test";

static uint32_t duty_max(void)
{
    return (1U << L293D_PWM_RES_BITS) - 1U;
}

static esp_err_t motor_set_speed(uint8_t speed_percent)
{
    if (speed_percent > 100) {
        speed_percent = 100;
    }

    const uint32_t duty = (duty_max() * speed_percent) / 100U;

    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, duty);
    if (err != ESP_OK) {
        return err;
    }

    return ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3);
}

static void motor_stop(void)
{
    ESP_ERROR_CHECK(motor_set_speed(0));
    gpio_set_level(L293D_IN1_GPIO, 0);
    gpio_set_level(L293D_IN2_GPIO, 0);
    ESP_LOGI(TAG, "Motor detenido");
}

static void motor_forward(uint8_t speed_percent)
{
    gpio_set_level(L293D_IN1_GPIO, 1);
    gpio_set_level(L293D_IN2_GPIO, 0);
    ESP_ERROR_CHECK(motor_set_speed(speed_percent));
    ESP_LOGI(TAG, "Motor adelante: %u%%", speed_percent);
}

static void motor_reverse(uint8_t speed_percent)
{
    gpio_set_level(L293D_IN1_GPIO, 0);
    gpio_set_level(L293D_IN2_GPIO, 1);
    ESP_ERROR_CHECK(motor_set_speed(speed_percent));
    ESP_LOGI(TAG, "Motor atras: %u%%", speed_percent);
}

esp_err_t l293d_motor_test_init(void)
{
    gpio_reset_pin(L293D_IN1_GPIO);
    gpio_reset_pin(L293D_IN2_GPIO);
    gpio_set_direction(L293D_IN1_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(L293D_IN2_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(L293D_IN1_GPIO, 0);
    gpio_set_level(L293D_IN2_GPIO, 0);

    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = L293D_PWM_RES_BITS,
        .timer_num = LEDC_TIMER_3,
        .freq_hz = L293D_PWM_FREQ_HZ,
    };

    esp_err_t err = ledc_timer_config(&timer_config);
    if (err != ESP_OK) {
        return err;
    }

    ledc_channel_config_t channel_config = {
        .gpio_num = L293D_ENABLE_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_3,
        .timer_sel = LEDC_TIMER_3,
    };

    err = ledc_channel_config(&channel_config);
    if (err != ESP_OK) {
        return err;
    }

    motor_stop();
    ESP_LOGI(TAG, "L293D listo: ENABLE GPIO%d, IN1 GPIO%d, IN2 GPIO%d",
             L293D_ENABLE_GPIO,
             L293D_IN1_GPIO,
             L293D_IN2_GPIO);

    return ESP_OK;
}

void l293d_motor_test_sequence(void)
{
    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(L293D_TEST_STOP_MS));

    motor_forward(L293D_TEST_SPEED_PERCENT);
    vTaskDelay(pdMS_TO_TICKS(L293D_TEST_RUN_MS));

    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(L293D_TEST_STOP_MS));

    motor_reverse(L293D_TEST_SPEED_PERCENT);
    vTaskDelay(pdMS_TO_TICKS(L293D_TEST_RUN_MS));

    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(L293D_TEST_STOP_MS));
}
