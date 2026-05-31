#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "motor_control.h"
#include "pwm_control.h"

static const char *TAG = "app_main";

void app_main(void)
{
    ESP_LOGI(TAG, "Firmware efecto fotoelectrico - pruebas iniciales");

    ESP_ERROR_CHECK(pwm_control_init());
    ESP_ERROR_CHECK(motor_control_init());

    while (true) {
        pwm_control_test_sequence();
        motor_control_test_sequence();
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
