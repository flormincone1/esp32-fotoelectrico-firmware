#include "cremallera.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "config.h"

static const char *TAG = "CREMALLERA";

#define PERIODO_CONTROL_MS  10

static const uint32_t tiempo_desde_home_ms[CANTIDAD_LEDS] = LEDS_TIEMPO_DESDE_HOME_MS;
static volatile bool cancelar = false;

esp_err_t cremallera_init(void)
{
    // El pull-up es externo (10 kohm a 3,3 V en la placa)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_HOME),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
    };
    esp_err_t ret = gpio_config(&io_conf);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al configurar el sensor de home");
        return ret;
    }

    ESP_LOGI(TAG, "Sensor de home en GPIO%d, estado actual: %s",
             PIN_HOME, cremallera_en_home() ? "EN HOME" : "fuera de home");
    return ESP_OK;
}

bool cremallera_en_home(void)
{
    return gpio_get_level(PIN_HOME) == HOME_NIVEL_ACTIVO;
}

void cremallera_cancelar(void)
{
    cancelar = true;
    motor_detener();
}

esp_err_t cremallera_homing(void)
{
    cancelar = false;

    if(cremallera_en_home()) {
        ESP_LOGI(TAG, "Ya esta en home");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Buscando home...");
    motor_mover(SENTIDO_HACIA_HOME, CREMALLERA_VELOCIDAD);

    uint32_t tiempo_ms = 0;
    while(!cremallera_en_home()) {
        if(cancelar) {
            motor_detener();
            ESP_LOGW(TAG, "Homing cancelado");
            return ESP_ERR_INVALID_STATE;
        }
        if(tiempo_ms >= HOMING_TIMEOUT_MS) {
            motor_detener();
            ESP_LOGE(TAG, "No se encontro el home en %d ms", HOMING_TIMEOUT_MS);
            return ESP_ERR_TIMEOUT;
        }
        vTaskDelay(pdMS_TO_TICKS(PERIODO_CONTROL_MS));
        tiempo_ms += PERIODO_CONTROL_MS;
    }

    motor_detener();
    ESP_LOGI(TAG, "Home encontrado en %lu ms", (unsigned long)tiempo_ms);
    return ESP_OK;
}

esp_err_t cremallera_ir_a_led(uint8_t led)
{
    if(led >= CANTIDAD_LEDS) {
        return ESP_ERR_INVALID_ARG;
    }

    // Siempre se parte del home: asi el error de posicion no se acumula
    esp_err_t ret = cremallera_homing();
    if(ret != ESP_OK) {
        return ret;
    }

    ESP_LOGI(TAG, "Moviendo al LED %d (%lu ms desde home)", led + 1, (unsigned long)tiempo_desde_home_ms[led]);
    motor_mover(SENTIDO_DESDE_HOME, CREMALLERA_VELOCIDAD);

    uint32_t tiempo_ms = 0;
    while(tiempo_ms < tiempo_desde_home_ms[led]) {
        if(cancelar) {
            motor_detener();
            ESP_LOGW(TAG, "Movimiento cancelado");
            return ESP_ERR_INVALID_STATE;
        }
        vTaskDelay(pdMS_TO_TICKS(PERIODO_CONTROL_MS));
        tiempo_ms += PERIODO_CONTROL_MS;
    }

    motor_detener();
    return ESP_OK;
}

void cremallera_pulso(motor_sentido_t sentido, uint32_t tiempo_ms)
{
    motor_mover(sentido, CREMALLERA_VELOCIDAD);
    vTaskDelay(pdMS_TO_TICKS(tiempo_ms));
    motor_detener();
}
