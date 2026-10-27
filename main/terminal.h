#ifndef TERMINAL_H
#define TERMINAL_H

#include "esp_err.h"

/**
 * @brief               Crea la tarea que lee el teclado del monitor serie (idf.py monitor)
 * @retval ESP_OK       Inicializacion correcta
 */
esp_err_t terminal_init(void);

#endif /* TERMINAL_H */
