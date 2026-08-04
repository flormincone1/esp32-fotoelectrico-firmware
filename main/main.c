#include <stdbool.h>

#include "esp_err.h"

#include "motor_terminal_control.h"

void app_main(void)
{
    ESP_ERROR_CHECK(motor_terminal_control_init());

    while (true) {
        motor_terminal_control_process();
    }
}
