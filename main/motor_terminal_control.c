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

static const char *TAG = "motor_terminal";

static uint8_t speed_percent;
static bool direction_forward = true;

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

static void motor_set_direction_pins(void)
{
    gpio_set_level(L293D_IN1_GPIO, direction_forward ? 1 : 0);
    gpio_set_level(L293D_IN2_GPIO, direction_forward ? 0 : 1);
}

static void motor_stop(void)
{
    ESP_ERROR_CHECK(motor_set_speed(0));
    gpio_set_level(L293D_IN1_GPIO, 0);
    gpio_set_level(L293D_IN2_GPIO, 0);
}

static void motor_apply_state(void)
{
    if (speed_percent == 0) {
        motor_stop();
        return;
    }

    motor_set_direction_pins();
    ESP_ERROR_CHECK(motor_set_speed(speed_percent));
}

static void log_state(void)
{
    ESP_LOGI(TAG, "Velocidad: %u%% | Direccion: %s",
             speed_percent,
             direction_forward ? "adelante" : "atras");
}

static void increase_speed(void)
{
    if (speed_percent <= 100 - MOTOR_TERMINAL_SPEED_STEP_PERCENT) {
        speed_percent += MOTOR_TERMINAL_SPEED_STEP_PERCENT;
    } else {
        speed_percent = 100;
    }

    motor_apply_state();
    log_state();
}

static void decrease_speed(void)
{
    if (speed_percent >= MOTOR_TERMINAL_SPEED_STEP_PERCENT) {
        speed_percent -= MOTOR_TERMINAL_SPEED_STEP_PERCENT;
    } else {
        speed_percent = 0;
    }

    motor_apply_state();
    log_state();
}

static void toggle_direction(void)
{
    if (speed_percent > 0) {
        ESP_ERROR_CHECK(motor_set_speed(0));
        vTaskDelay(pdMS_TO_TICKS(MOTOR_TERMINAL_REVERSE_DELAY_MS));
    }

    direction_forward = !direction_forward;
    motor_apply_state();
    log_state();
}

static void handle_key(char key)
{
    switch (key) {
    case '+':
        increase_speed();
        break;

    case '-':
        decrease_speed();
        break;

    case ' ':
        toggle_direction();
        break;

    case 's':
    case 'S':
        speed_percent = 0;
        motor_stop();
        ESP_LOGI(TAG, "Motor detenido");
        break;

    default:
        break;
    }
}

esp_err_t motor_terminal_control_init(void)
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

    if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK) == -1) {
        return ESP_FAIL;
    }

    speed_percent = 0;
    direction_forward = true;
    motor_stop();

    ESP_LOGI(TAG, "Control listo: +5%% | -5%% | ESPACIO cambia direccion | S detiene");
    log_state();

    return ESP_OK;
}

void motor_terminal_control_process(void)
{
    char key;

    if (read(STDIN_FILENO, &key, 1) == 1) {
        handle_key(key);
    }

    vTaskDelay(pdMS_TO_TICKS(MOTOR_TERMINAL_POLL_DELAY_MS));
}
