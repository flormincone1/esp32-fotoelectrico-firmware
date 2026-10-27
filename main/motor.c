#include "motor.h"

#include <stdbool.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "config.h"

static const char *TAG = "MOTOR";

#define MOTOR_TIMER         LEDC_TIMER_1
#define MOTOR_CANAL         LEDC_CHANNEL_1
#define MOTOR_RESOLUCION    LEDC_TIMER_10_BIT
#define MOTOR_DUTY_MAX      1023

static bool en_marcha = false;
static motor_sentido_t sentido_actual = MOTOR_ADELANTE;

/* Escribe el duty del PWM. Devuelve el error sin abortar */
static esp_err_t motor_set_duty(uint8_t porcentaje)
{
    if(porcentaje > 100) {
        porcentaje = 100;
    }

    uint32_t duty = (MOTOR_DUTY_MAX * porcentaje) / 100;
    esp_err_t ret = ledc_set_duty(LEDC_LOW_SPEED_MODE, MOTOR_CANAL, duty);
    if(ret != ESP_OK) {
        return ret;
    }
    return ledc_update_duty(LEDC_LOW_SPEED_MODE, MOTOR_CANAL);
}

/* Fija el sentido en DIR y su complemento en DIR_N (para el L293D) */
static void motor_set_sentido(motor_sentido_t sentido)
{
    int nivel = (sentido == MOTOR_ADELANTE) ? 1 : 0;
    if(MOTOR_DIR_INVERTIDO) {
        nivel = !nivel;
    }

    gpio_set_level(PIN_MOTOR_DIR, nivel);
    gpio_set_level(PIN_MOTOR_DIR_N, !nivel);
    sentido_actual = sentido;
}

esp_err_t motor_init(void)
{
    esp_err_t ret;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_MOTOR_DIR) | (1ULL << PIN_MOTOR_DIR_N),
        .mode = GPIO_MODE_OUTPUT,
    };
    ret = gpio_config(&io_conf);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al configurar los pines de direccion");
        return ret;
    }
    motor_set_sentido(MOTOR_ADELANTE);

    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = MOTOR_RESOLUCION,
        .timer_num = MOTOR_TIMER,
        .freq_hz = MOTOR_PWM_FRECUENCIA_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ret = ledc_timer_config(&timer);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al configurar el timer del PWM");
        return ret;
    }

    ledc_channel_config_t canal = {
        .gpio_num = PIN_MOTOR_PWM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = MOTOR_CANAL,
        .timer_sel = MOTOR_TIMER,
        .duty = 0,                                  // Arranca detenido
        .flags.output_invert = MOTOR_PWM_INVERTIDO,
    };
    ret = ledc_channel_config(&canal);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al configurar el canal del PWM");
        return ret;
    }

    en_marcha = false;
    ESP_LOGI(TAG, "Puente H: PWM GPIO%d, DIR GPIO%d, DIR_N GPIO%d",
             PIN_MOTOR_PWM, PIN_MOTOR_DIR, PIN_MOTOR_DIR_N);
    return ESP_OK;
}

void motor_mover(motor_sentido_t sentido, uint8_t velocidad)
{
    // Nunca se cambia DIR con el puente conduciendo: se apaga, se espera y recien ahi se cambia
    if(en_marcha && sentido != sentido_actual) {
        motor_detener();
        vTaskDelay(pdMS_TO_TICKS(MOTOR_TIEMPO_MUERTO_MS));
    }

    motor_set_sentido(sentido);

    if(motor_set_duty(velocidad) != ESP_OK) {
        ESP_LOGE(TAG, "Error al fijar la velocidad. Se detiene el motor");
        motor_detener();
        return;
    }
    en_marcha = (velocidad > 0);
}

void motor_detener(void)
{
    // Sin ESP_ERROR_CHECK: si esto fallara y reiniciara el ESP32, el motor podria quedar encendido
    if(motor_set_duty(0) != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo detener el motor");
    }
    en_marcha = false;
}
