#include "led.h"

#include "driver/ledc.h"
#include "esp_log.h"

#include "config.h"

static const char *TAG = "LED";

#define LED_TIMER       LEDC_TIMER_0
#define LED_CANAL       LEDC_CHANNEL_0
#define LED_RESOLUCION  LEDC_TIMER_10_BIT
#define LED_DUTY_MAX    1023

esp_err_t led_init(void)
{
    esp_err_t ret;

    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LED_RESOLUCION,
        .timer_num = LED_TIMER,
        .freq_hz = LED_PWM_FRECUENCIA_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ret = ledc_timer_config(&timer);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al configurar el timer del PWM");
        return ret;
    }

    ledc_channel_config_t canal = {
        .gpio_num = PIN_LED_PWM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LED_CANAL,
        .timer_sel = LED_TIMER,
        .duty = 0,                                  // Arranca apagado
        .flags.output_invert = LED_PWM_INVERTIDO,
    };
    ret = ledc_channel_config(&canal);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al configurar el canal del PWM");
        return ret;
    }

    ESP_LOGI(TAG, "PWM de LED en GPIO%d, %d Hz", PIN_LED_PWM, LED_PWM_FRECUENCIA_HZ);
    return ESP_OK;
}

esp_err_t led_set_intensidad(uint8_t porcentaje)
{
    if(porcentaje > 100) {
        porcentaje = 100;
    }

    uint32_t duty = (LED_DUTY_MAX * porcentaje) / 100;
    if(porcentaje == 100) {
        duty = LED_DUTY_MAX + 1;    // Duty completo: salida siempre en alto, sin pulsos
    }

    esp_err_t ret = ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_CANAL, duty);
    if(ret != ESP_OK) {
        return ret;
    }
    return ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_CANAL);
}

void led_apagar(void)
{
    // Sin ESP_ERROR_CHECK: apagar nunca debe reiniciar el ESP32
    if(led_set_intensidad(0) != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo apagar el LED");
    }
}
