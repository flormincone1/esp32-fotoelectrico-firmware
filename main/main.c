#include <stdint.h>
#include <stdbool.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "app_main";

#define ONBOARD_LED_GPIO 2
#define LOOP_DELAY_MS    5000

void app_main(void)
{
    uint32_t counter = 0;
    bool led_state = false;

    gpio_reset_pin(ONBOARD_LED_GPIO);
    gpio_set_direction(ONBOARD_LED_GPIO, GPIO_MODE_OUTPUT);

    ESP_LOGI(TAG, "Hola mundo desde ESP32-WROOM-32");
    ESP_LOGI(TAG, "Proyecto: digitalizacion del experimento de efecto fotoelectrico");
    ESP_LOGI(TAG, "Blinky configurado en GPIO%d", ONBOARD_LED_GPIO);

    while (true) {
        led_state = !led_state;
        gpio_set_level(ONBOARD_LED_GPIO, led_state);

        ESP_LOGI(TAG, "Contador de vida: %lu | LED: %s",
                 (unsigned long)counter++,
                 led_state ? "ON" : "OFF");

        vTaskDelay(pdMS_TO_TICKS(LOOP_DELAY_MS));
    }
}
