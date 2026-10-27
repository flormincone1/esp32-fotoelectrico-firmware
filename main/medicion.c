#include "medicion.h"

#include <stdbool.h>

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#include "config.h"

static const char *TAG = "MEDICION";

static adc_oneshot_unit_handle_t adc = NULL;
static adc_cali_handle_t calibracion = NULL;

esp_err_t medicion_init(void)
{
    esp_err_t ret;

    // Se usa el ADC1 porque el ADC2 no funciona mientras el Wi-Fi esta activo
    adc_oneshot_unit_init_cfg_t unidad = {
        .unit_id = ADC_UNIT_1,
    };
    ret = adc_oneshot_new_unit(&unidad, &adc);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al iniciar el ADC1");
        return ret;
    }

    // Atenuacion de 12 dB: permite medir hasta ~3,1 V
    adc_oneshot_chan_cfg_t canal = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ret = adc_oneshot_config_channel(adc, ADC_CANAL_CORRIENTE, &canal);
    if(ret != ESP_OK) {
        return ret;
    }
    ret = adc_oneshot_config_channel(adc, ADC_CANAL_FRENADO, &canal);
    if(ret != ESP_OK) {
        return ret;
    }

    // Calibracion de fabrica (eFuse) para convertir lecturas a mV
    adc_cali_line_fitting_config_t cali = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
        .default_vref = 1100,   // Solo se usa si el chip no tiene calibracion grabada
    };
    ret = adc_cali_create_scheme_line_fitting(&cali, &calibracion);
    if(ret != ESP_OK) {
        ESP_LOGW(TAG, "Sin calibracion del ADC: se usara una conversion aproximada");
        calibracion = NULL;
    }

    ESP_LOGI(TAG, "ADC1 listo: corriente en GPIO32, frenado en GPIO33");
    return ESP_OK;
}

/* Promedia varias lecturas de un canal y devuelve el resultado en mV */
static float leer_mv(adc_channel_t canal)
{
    int suma_mv = 0;
    int validas = 0;

    for(int i = 0; i < MUESTRAS_POR_PUNTO; i++) {
        int crudo = 0;
        if(adc_oneshot_read(adc, canal, &crudo) != ESP_OK) {
            continue;
        }

        int mv = 0;
        if(calibracion != NULL) {
            adc_cali_raw_to_voltage(calibracion, crudo, &mv);
        }
        else {
            mv = crudo * 3100 / 4095;
        }
        suma_mv += mv;
        validas++;
    }

    if(validas == 0) {
        ESP_LOGE(TAG, "No se pudo leer el canal %d", canal);
        return 0.0f;
    }
    return (float)suma_mv / validas;
}

float medicion_leer_corriente_na(void)
{
    return leer_mv(ADC_CANAL_CORRIENTE) / CORRIENTE_MV_POR_NA;
}

float medicion_leer_frenado_v(void)
{
    return leer_mv(ADC_CANAL_FRENADO) / 1000.0f;
}
