#ifndef WIFI_AP_H
#define WIFI_AP_H

#include "esp_err.h"

/**
 * @brief               Crea una red Wi-Fi propia (modo punto de acceso)
 * @note                Nombre y clave en config.h. El ESP32 queda en 192.168.4.1
 * @retval ESP_OK       Red creada
 */
esp_err_t wifi_ap_init(void);

#endif /* WIFI_AP_H */
