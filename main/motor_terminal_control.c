#include "motor_terminal_control.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_config.h"
#include "motor_terminal_config.h"
#include "pwm_control.h"

static const char *TAG = "motor_terminal";

static uint8_t led_intensity_percent;
static bool next_direction_forward = true;
static bool motor_running;
static TickType_t motor_stop_tick;

static uint32_t duty_max(void)
{
    return (1U << L293D_PWM_RES_BITS) - 1U;
}

static esp_err_t motor_set_speed(uint8_t percent)
{
    const uint32_t duty = (duty_max() * percent) / 100U;

    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, duty);
    if (err != ESP_OK) {
        return err;
    }

    return ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3);
}

static void motor_set_direction(bool direction_forward)
{
    gpio_set_level(L293D_IN1_GPIO, direction_forward ? 1 : 0);
    gpio_set_level(L293D_IN2_GPIO, direction_forward ? 0 : 1);
}

static void motor_stop(void)
{
    ESP_ERROR_CHECK(motor_set_speed(0));
    gpio_set_level(L293D_IN1_GPIO, 0);
    gpio_set_level(L293D_IN2_GPIO, 0);
    motor_running = false;
}

static void motor_start_move(void)
{
    if (motor_running) {
        motor_stop();
        vTaskDelay(pdMS_TO_TICKS(MOTOR_DIRECTION_CHANGE_DELAY_MS));
    }

    motor_set_direction(next_direction_forward);
    ESP_ERROR_CHECK(motor_set_speed(L293D_TEST_SPEED_PERCENT));

    motor_running = true;
    motor_stop_tick = xTaskGetTickCount() + pdMS_TO_TICKS(L293D_TEST_RUN_MS);

    ESP_LOGI(TAG, "Motor %s: %d%% durante %d ms",
             next_direction_forward ? "adelante" : "atras",
             L293D_TEST_SPEED_PERCENT,
             L293D_TEST_RUN_MS);

    next_direction_forward = !next_direction_forward;
}

static void motor_update(void)
{
    if (motor_running && (int32_t)(xTaskGetTickCount() - motor_stop_tick) >= 0) {
        motor_stop();
        ESP_LOGI(TAG, "Motor detenido: movimiento finalizado");
    }
}

static void increase_led_intensity(void)
{
    if (led_intensity_percent <= 100 - LED_PWM_STEP_PERCENT) {
        led_intensity_percent += LED_PWM_STEP_PERCENT;
    } else {
        led_intensity_percent = 100;
    }

    ESP_ERROR_CHECK(pwm_control_set_duty_percent(led_intensity_percent));
    ESP_LOGI(TAG, "Intensidad LED: %u%%", led_intensity_percent);
}

static void decrease_led_intensity(void)
{
    if (led_intensity_percent >= LED_PWM_STEP_PERCENT) {
        led_intensity_percent -= LED_PWM_STEP_PERCENT;
    } else {
        led_intensity_percent = 0;
    }

    ESP_ERROR_CHECK(pwm_control_set_duty_percent(led_intensity_percent));
    ESP_LOGI(TAG, "Intensidad LED: %u%%", led_intensity_percent);
}

static void handle_key(char key)
{
    switch (key) {
    case '+':
        increase_led_intensity();
        break;

    case '-':
        decrease_led_intensity();
        break;

    case ' ':
        motor_start_move();
        break;

    case 's':
    case 'S':
        motor_stop();
        ESP_LOGI(TAG, "Motor detenido por el usuario");
        break;

    default:
        break;
    }
}

esp_err_t motor_terminal_control_init(void)
{
    esp_err_t err = pwm_control_init();
    if (err != ESP_OK) {
        return err;
    }

    err = pwm_control_set_duty_percent(0);
    if (err != ESP_OK) {
        return err;
    }

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

    err = ledc_timer_config(&timer_config);
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

    if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK) == -1) {
        return ESP_FAIL;
    }

    led_intensity_percent = 0;
    next_direction_forward = true;
    motor_stop();

    ESP_LOGI(TAG, "+/- intensidad LED en pasos de %d%%", LED_PWM_STEP_PERCENT);
    ESP_LOGI(TAG, "ESPACIO mueve el motor %d ms al %d%% y alterna direccion",
             L293D_TEST_RUN_MS,
             L293D_TEST_SPEED_PERCENT);
    ESP_LOGI(TAG, "S detiene el motor");

    return ESP_OK;
}

void motor_terminal_control_process(void)
{
    char key;

    if (read(STDIN_FILENO, &key, 1) == 1) {
        handle_key(key);
    }

    motor_update();
    vTaskDelay(pdMS_TO_TICKS(MOTOR_TERMINAL_POLL_DELAY_MS));
}
