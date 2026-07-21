#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "motor_control.h"

void app_main(void)
{
    ESP_ERROR_CHECK(motor_control_init());

    while (true) {
        motor_control_test_sequence();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
