#ifndef FRENADO_H
#define FRENADO_H

#include <stdint.h>

#include "esp_err.h"

/**
 * @brief               Configura el DAC y deja el potencial de frenado en 0 V
 * @retval ESP_OK       Inicializacion correcta
 */
esp_err_t frenado_init(void);

/**
 * @brief               Fija la tension de salida del DAC
 * @note                El amplificador la invierte y la aplica a la fotocelula como frenado.
 *                      La tension real aplicada se mide con medicion_leer_frenado_v()
 * @param[in] mv        Tension pedida en mV (0 a DAC_TENSION_MAXIMA_MV)
 * @retval ESP_OK       Cambio correcto
 */
esp_err_t frenado_set_mv(uint16_t mv);

#endif /* FRENADO_H */
