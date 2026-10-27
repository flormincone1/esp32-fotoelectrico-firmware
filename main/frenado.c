#include "frenado.h"

#include "driver/dac_oneshot.h"
#include "esp_log.h"

#include "config.h"

static const char *TAG = "FRENADO";

static dac_oneshot_handle_t dac = NULL;

esp_err_t frenado_init(void)
{
    dac_oneshot_config_t config = {
        .chan_id = DAC_CANAL_FRENADO,
    };
    esp_err_t ret = dac_oneshot_new_channel(&config, &dac);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al configurar el DAC");
        return ret;
    }

    ESP_LOGI(TAG, "DAC listo en GPIO25");
    return frenado_set_mv(0);
}

esp_err_t frenado_set_mv(uint16_t mv)
{
    if(dac == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    if(mv > DAC_TENSION_MAXIMA_MV) {
        mv = DAC_TENSION_MAXIMA_MV;
    }

    // El DAC es de 8 bits: 0 -> 0 V, 255 -> DAC_TENSION_MAXIMA_MV
    uint8_t valor = (uint8_t)((mv * 255UL + DAC_TENSION_MAXIMA_MV / 2) / DAC_TENSION_MAXIMA_MV);
    return dac_oneshot_output_voltage(dac, valor);
}
