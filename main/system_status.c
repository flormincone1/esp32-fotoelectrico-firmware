#include "system_status.h"

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_log.h"

#include "app_config.h"

static const char *TAG = "system_status";

static uint32_t counter = 0;
static bool led_state = false;

void system_status_init(void)
{
    gpio_reset_pin(APP_ONBOARD_LED_GPIO);
    gpio_set_direction(APP_ONBOARD_LED_GPIO, GPIO_MODE_OUTPUT);

    ESP_LOGI(TAG, "Hola mundo desde ESP32-WROOM-32");
    ESP_LOGI(TAG, "Proyecto: digitalizacion del experimento de efecto fotoelectrico");
    ESP_LOGI(TAG, "Blinky configurado en GPIO%d", APP_ONBOARD_LED_GPIO);
}

void system_status_update(void)
{
    led_state = !led_state;
    gpio_set_level(APP_ONBOARD_LED_GPIO, led_state);

    ESP_LOGI(TAG, "Contador de vida: %lu | LED: %s",
             (unsigned long)counter++,
             led_state ? "ON" : "OFF");
}
