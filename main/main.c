#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "pwm_control.h"
#include "system_status.h"

void app_main(void)
{
    system_status_init();
    pwm_control_init();

    while (true) {
        pwm_control_test_sequence();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
