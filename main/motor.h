#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

#include "esp_err.h"

typedef enum {
    MOTOR_ADELANTE = 0,
    MOTOR_ATRAS,
} motor_sentido_t;

/**
 * @brief               Configura los pines DIR y PWM del puente H y deja el motor detenido
 * @retval ESP_OK       Inicializacion correcta
 * @retval Other        Error al configurar GPIO o LEDC
 */
esp_err_t motor_init(void);

/**
 * @brief               Pone el motor en marcha
 * @note                Si hay que cambiar el sentido, primero lo detiene y espera MOTOR_TIEMPO_MUERTO_MS
 * @param[in] sentido   MOTOR_ADELANTE o MOTOR_ATRAS
 * @param[in] velocidad Ciclo de trabajo de 0 a 100 %
 */
void motor_mover(motor_sentido_t sentido, uint8_t velocidad);

/**
 * @brief               Detiene el motor. Se puede llamar desde cualquier tarea
 */
void motor_detener(void);

#endif /* MOTOR_H */
