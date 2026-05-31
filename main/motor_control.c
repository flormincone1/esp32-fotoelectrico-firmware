#include "motor_control.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_config.h"

static const char *TAG = "motor_control";

static uint32_t duty_max(void)
{
    return (1U << APP_PWM_RES_BITS) - 1U;
}

esp_err_t motor_control_init(void)
{
    gpio_config_t dir_config = {
        .pin_bit_mask = 1ULL << APP_MOTOR_DIR_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t err = gpio_config(&dir_config);
    if (err != ESP_OK) {
        return err;
    }

    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = APP_PWM_RES_BITS,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = APP_PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = false,
    };

    err = ledc_timer_config(&timer_config);
    if (err != ESP_OK) {
        return err;
    }

    ledc_channel_config_t channel_config = {
        .gpio_num = APP_MOTOR_PWM_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0,
        .hpoint = 0,
        .flags.output_invert = 0,
    };

    err = ledc_channel_config(&channel_config);
    if (err != ESP_OK) {
        return err;
    }

    err = motor_control_stop();
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Motor iniciado: DIR GPIO %d, PWM GPIO %d", APP_MOTOR_DIR_GPIO, APP_MOTOR_PWM_GPIO);
    }
    return err;
}

esp_err_t motor_control_set_direction(motor_direction_t direction)
{
    const uint32_t level = (direction == MOTOR_DIR_FORWARD) ? 0 : 1;
    ESP_LOGI(TAG, "Direccion motor: %s", direction == MOTOR_DIR_FORWARD ? "forward" : "reverse");
    return gpio_set_level(APP_MOTOR_DIR_GPIO, level);
}

esp_err_t motor_control_set_speed_percent(uint8_t speed_percent)
{
    if (speed_percent > 100) {
        speed_percent = 100;
    }

    const uint32_t duty = (duty_max() * speed_percent) / 100U;

    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
    if (err != ESP_OK) {
        return err;
    }

    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Velocidad motor: %u%%", speed_percent);
    }
    return err;
}

esp_err_t motor_control_stop(void)
{
    return motor_control_set_speed_percent(0);
}

void motor_control_test_sequence(void)
{
    ESP_LOGI(TAG, "Inicio prueba motor");

    motor_control_stop();
    vTaskDelay(pdMS_TO_TICKS(1000));

    motor_control_set_direction(MOTOR_DIR_FORWARD);
    motor_control_set_speed_percent(30);
    vTaskDelay(pdMS_TO_TICKS(APP_TEST_DELAY_MS));

    motor_control_stop();
    vTaskDelay(pdMS_TO_TICKS(1000));

    motor_control_set_direction(MOTOR_DIR_REVERSE);
    motor_control_set_speed_percent(30);
    vTaskDelay(pdMS_TO_TICKS(APP_TEST_DELAY_MS));

    motor_control_stop();
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "Fin prueba motor");
}
