#include "wifi_ap.h"

#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "config.h"

static const char *TAG = "WIFI";

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if(event_id == WIFI_EVENT_AP_STACONNECTED) {
        ESP_LOGI(TAG, "Se conecto un dispositivo");
    }
    else if(event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        ESP_LOGI(TAG, "Se desconecto un dispositivo");
    }
}

esp_err_t wifi_ap_init(void)
{
    // El Wi-Fi guarda su configuracion en la memoria NVS
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if(ret != ESP_OK) {
        return ret;
    }

    ESP_ERROR_CHECK(esp_netif_init());                  // Pila TCP/IP
    ESP_ERROR_CHECK(esp_event_loop_create_default());   // Loop de eventos
    esp_netif_create_default_wifi_ap();                 // Interfaz de red en modo AP

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .password = WIFI_PASSWORD,
            .channel = WIFI_CANAL,
            .max_connection = WIFI_MAX_CONEXIONES,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Red \"%s\" creada. Abrir http://192.168.4.1", WIFI_SSID);
    return ESP_OK;
}
