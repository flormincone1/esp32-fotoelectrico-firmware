#ifndef LED_H
#define LED_H

#include <stdint.h>

#include "esp_err.h"

/**
 * @brief               Configura el PWM de los LED y los deja apagados
 * @retval ESP_OK       Inicializacion correcta
 * @retval Other        Error al configurar el periferico LEDC
 */
esp_err_t led_init(void);

/**
 * @brief               Fija la intensidad de los LED
 * @param[in] porcentaje Ciclo de trabajo de 0 (apagado) a 100 (luz continua)
 * @retval ESP_OK       Cambio correcto
 */
esp_err_t led_set_intensidad(uint8_t porcentaje);

/**
 * @brief               Apaga los LED
 */
void led_apagar(void);

#endif /* LED_H */
