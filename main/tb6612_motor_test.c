#include "tb6612_motor_test.h"

#include <stdint.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_config.h"

static const char *TAG = "tb6612_test";

static uint32_t duty_max(void)
{
    return (1U << TB6612_PWM_RES_BITS) - 1U;
}

static void motor_set_speed(uint8_t speed_percent)
{
    if (speed_percent > 100) {
        speed_percent = 100;
    }

    uint32_t duty = (duty_max() * speed_percent) / 100U;

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
}

static void motor_stop(void)
{
    gpio_set_level(TB6612_AIN1_GPIO, 0);
    gpio_set_level(TB6612_AIN2_GPIO, 0);
    motor_set_speed(0);

    ESP_LOGI(TAG, "Motor detenido");
}

static void motor_forward(uint8_t speed_percent)
{
    gpio_set_level(TB6612_AIN1_GPIO, 1);
    gpio_set_level(TB6612_AIN2_GPIO, 0);
    motor_set_speed(speed_percent);

    ESP_LOGI(TAG, "Motor adelante - velocidad %u%%", speed_percent);
}

static void motor_reverse(uint8_t speed_percent)
{
    gpio_set_level(TB6612_AIN1_GPIO, 0);
    gpio_set_level(TB6612_AIN2_GPIO, 1);
    motor_set_speed(speed_percent);

    ESP_LOGI(TAG, "Motor atras - velocidad %u%%", speed_percent);
}

void tb6612_motor_test_init(void)
{
    gpio_reset_pin(TB6612_AIN1_GPIO);
    gpio_reset_pin(TB6612_AIN2_GPIO);
    gpio_reset_pin(TB6612_STBY_GPIO);

    gpio_set_direction(TB6612_AIN1_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(TB6612_AIN2_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(TB6612_STBY_GPIO, GPIO_MODE_OUTPUT);

    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = TB6612_PWM_RES_BITS,
        .timer_num = LEDC_TIMER_2,
        .freq_hz = TB6612_PWM_FREQ_HZ,
    };

    ledc_timer_config(&timer_config);

    ledc_channel_config_t channel_config = {
        .gpio_num = TB6612_PWMA_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_2,
        .timer_sel = LEDC_TIMER_2,
    };

    ledc_channel_config(&channel_config);

    gpio_set_level(TB6612_STBY_GPIO, 1);
    motor_stop();

    ESP_LOGI(TAG, "TB6612FNG listo: PWMA GPIO%d, AIN1 GPIO%d, AIN2 GPIO%d, STBY GPIO%d",
             TB6612_PWMA_GPIO,
             TB6612_AIN1_GPIO,
             TB6612_AIN2_GPIO,
             TB6612_STBY_GPIO);
}

void tb6612_motor_test_sequence(void)
{
    motor_forward(30);
    vTaskDelay(pdMS_TO_TICKS(TB6612_TEST_DELAY_MS));

    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(TB6612_TEST_DELAY_MS));

    motor_reverse(30);
    vTaskDelay(pdMS_TO_TICKS(TB6612_TEST_DELAY_MS));

    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(TB6612_TEST_DELAY_MS));
}
