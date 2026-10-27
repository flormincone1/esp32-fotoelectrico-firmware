/*
 * Firmware del experimento de efecto fotoelectrico - UTN FRA
 *
 * Tareas:
 *   task_experimento  maneja todo el hardware (LED, motor, DAC, ADC) segun los comandos que recibe
 *   task_terminal     lee el teclado del monitor serie y envia comandos
 *   servidor web      (tarea propia de ESP-IDF) pagina y API en http://192.168.4.1
 */

#include "esp_log.h"

#include "cremallera.h"
#include "experimento.h"
#include "frenado.h"
#include "led.h"
#include "medicion.h"
#include "motor.h"
#include "servidor_web.h"
#include "terminal.h"
#include "wifi_ap.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    // Primero las salidas: asi el motor y los LED quedan apagados cuanto antes
    ESP_LOGI(TAG, "Inicializando motor y LED");
    ESP_ERROR_CHECK(motor_init());
    ESP_ERROR_CHECK(led_init());

    ESP_LOGI(TAG, "Inicializando frenado (DAC) y medicion (ADC)");
    ESP_ERROR_CHECK(frenado_init());
    ESP_ERROR_CHECK(medicion_init());

    ESP_LOGI(TAG, "Inicializando sensor de home");
    ESP_ERROR_CHECK(cremallera_init());

    ESP_LOGI(TAG, "Inicializando experimento");
    ESP_ERROR_CHECK(experimento_init());

    ESP_LOGI(TAG, "Inicializando terminal");
    ESP_ERROR_CHECK(terminal_init());

    ESP_LOGI(TAG, "Inicializando Wi-Fi");
    ESP_ERROR_CHECK(wifi_ap_init());

    ESP_LOGI(TAG, "Inicializando servidor web");
    ESP_ERROR_CHECK(servidor_web_init());

    ESP_LOGI(TAG, "Sistema listo. Escribir '?' en el monitor para ver los comandos");
}
