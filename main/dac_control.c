#include "dac_control.h"

#include "driver/dac_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dac_test_config.h"

static const char *TAG = "dac_control";
static dac_oneshot_handle_t dac_handle;

esp_err_t dac_control_init(void)
{
    dac_oneshot_config_t channel_config = {
        .chan_id = DAC_TEST_CHANNEL,
    };

    esp_err_t err = dac_oneshot_new_channel(&channel_config, &dac_handle);
    if (err != ESP_OK) {
        return err;
    }

    err = dac_control_set_raw(0);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "DAC iniciado en GPIO%d", DAC_TEST_OUTPUT_GPIO);
    }

    return err;
}

esp_err_t dac_control_set_raw(uint8_t value)
{
    return dac_oneshot_output_voltage(dac_handle, value);
}

esp_err_t dac_control_set_percent(uint8_t percent)
{
    if (percent > 100) {
        percent = 100;
    }

    const uint8_t value = (255U * percent + 50U) / 100U;

    esp_err_t err = dac_control_set_raw(value);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Salida DAC: %u%%, valor digital %u", percent, value);
    }

    return err;
}

void dac_control_test_sequence(void)
{
    const uint8_t levels_percent[] = {0, 25, 50, 75, 100};

    for (uint8_t i = 0; i < sizeof(levels_percent); ++i) {
        ESP_ERROR_CHECK(dac_control_set_percent(levels_percent[i]));
        vTaskDelay(pdMS_TO_TICKS(DAC_TEST_STEP_DELAY_MS));
    }
}
