#ifndef MEDICION_H
#define MEDICION_H

#include "esp_err.h"

/**
 * @brief               Configura el ADC1 y su calibracion para los canales de corriente y frenado
 * @retval ESP_OK       Inicializacion correcta
 */
esp_err_t medicion_init(void);

/**
 * @brief               Mide la fotocorriente promediando MUESTRAS_POR_PUNTO lecturas
 * @return float        Corriente en nA
 */
float medicion_leer_corriente_na(void);

/**
 * @brief               Mide el potencial de frenado realmente aplicado (salida U0 del amplificador)
 * @return float        Tension en V
 */
float medicion_leer_frenado_v(void);

#endif /* MEDICION_H */
