#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "app_main";

void app_main(void)
{
    uint32_t counter = 0;

    ESP_LOGI(TAG, "Hola mundo desde ESP32-WROOM-32");
    ESP_LOGI(TAG, "Proyecto: digitalizacion del experimento de efecto fotoelectrico");

    while (true) {
        ESP_LOGI(TAG, "Contador de vida: %lu", (unsigned long)counter++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
