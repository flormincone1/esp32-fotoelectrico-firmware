#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "l293d_motor_test.h"

void app_main(void)
{
    ESP_ERROR_CHECK(l293d_motor_test_init());

    while (true) {
        l293d_motor_test_sequence();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
