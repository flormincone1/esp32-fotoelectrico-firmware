#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_config.h"
#include "system_status.h"

void app_main(void)
{
    system_status_init();

    while (true) {
        system_status_update();
        vTaskDelay(pdMS_TO_TICKS(APP_STATUS_DELAY_MS));
    }
}
