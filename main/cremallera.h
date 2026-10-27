#ifndef CREMALLERA_H
#define CREMALLERA_H

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#include "motor.h"

/**
 * @brief               Configura el sensor de home
 * @retval ESP_OK       Inicializacion correcta
 */
esp_err_t cremallera_init(void);

/**
 * @brief               Indica si la cremallera esta en la posicion inicial
 * @return bool         true si el sensor de home esta activo
 */
bool cremallera_en_home(void);

/**
 * @brief               Lleva la cremallera hasta el sensor de home
 * @retval ESP_OK       Llego al home
 * @retval ESP_ERR_TIMEOUT  No encontro el home en HOMING_TIMEOUT_MS
 * @retval ESP_ERR_INVALID_STATE  Movimiento cancelado con cremallera_cancelar()
 */
esp_err_t cremallera_homing(void);

/**
 * @brief               Hace homing y despues deja el LED pedido frente a la fotocelula
 * @param[in] led       Indice del LED, de 0 a CANTIDAD_LEDS - 1
 * @retval ESP_OK       Movimiento completo
 * @retval Other        Error de homing, indice invalido o movimiento cancelado
 */
esp_err_t cremallera_ir_a_led(uint8_t led);

/**
 * @brief               Mueve el motor un tiempo fijo (para calibrar las posiciones)
 * @param[in] sentido   MOTOR_ADELANTE o MOTOR_ATRAS
 * @param[in] tiempo_ms Duracion del movimiento
 */
void cremallera_pulso(motor_sentido_t sentido, uint32_t tiempo_ms);

/**
 * @brief               Pide cortar el movimiento en curso. Se puede llamar desde cualquier tarea
 */
void cremallera_cancelar(void);

#endif /* CREMALLERA_H */
