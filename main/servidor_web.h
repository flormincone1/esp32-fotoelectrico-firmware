#ifndef SERVIDOR_WEB_H
#define SERVIDOR_WEB_H

#include "esp_err.h"

/**
 * @brief               Inicia el servidor HTTP con la pagina y la API REST
 * @note                Rutas: /  /api/estado  /api/comando  /api/resultados  /api/resultados.csv
 * @retval ESP_OK       Servidor iniciado
 */
esp_err_t servidor_web_init(void);

#endif /* SERVIDOR_WEB_H */
