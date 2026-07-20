#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tb6612_motor_test.h"

void app_main(void)
{
    tb6612_motor_test_init();

    while (true) {
        tb6612_motor_test_sequence();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
