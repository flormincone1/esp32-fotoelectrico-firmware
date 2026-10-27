#include "servidor_web.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_http_server.h"
#include "esp_log.h"

#include "experimento.h"

static const char *TAG = "WEB";

/* Pagina web: main/web/index.html, incluida en el binario por CMakeLists.txt */
extern const char index_html_inicio[] asm("_binary_index_html_start");
extern const char index_html_fin[] asm("_binary_index_html_end");

static resultados_t copia_resultados;   // static: es grande para la pila del servidor

static const char *texto_corte(resultado_corte_t corte)
{
    switch(corte) {
    case CORTE_OK:              return "ok";
    case CORTE_SIN_FOTOEMISION: return "sin_fotoemision";
    case CORTE_FUERA_DE_RANGO:  return "fuera_de_rango";
    }
    return "?";
}

/* GET / */
static esp_err_t pagina_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    return httpd_resp_send(req, index_html_inicio, index_html_fin - index_html_inicio - 1);
}

/* GET /api/estado */
static esp_err_t estado_handler(httpd_req_t *req)
{
    estado_experimento_t e;
    experimento_obtener_estado(&e);

    char json[384];
    snprintf(json, sizeof(json),
             "{\"estado\":\"%s\",\"mensaje\":\"%s\",\"led\":%d,\"intensidad\":%d,\"frenado_mv\":%d,"
             "\"punto\":%d,\"puntos\":%d,\"u\":%.4f,\"i\":%.4f,\"home\":%s}",
             experimento_texto_estado(e.estado), e.mensaje, e.led_actual, e.intensidad, e.frenado_mv,
             e.punto_actual, e.puntos_totales, e.frenado_v, e.corriente_na, e.en_home ? "true" : "false");

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, json);
}

/* POST /api/comando?accion=...&valor=... */
static esp_err_t comando_handler(httpd_req_t *req)
{
    char query[64];
    char accion[24] = "";
    char valor_txt[12] = "0";

    if(httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "accion", accion, sizeof(accion));
        httpd_query_key_value(query, "valor", valor_txt, sizeof(valor_txt));
    }
    int32_t valor = atoi(valor_txt);
    bool ok = true;

    if(strcmp(accion, "detener") == 0)                  experimento_detener();
    else if(strcmp(accion, "homing") == 0)              ok = experimento_enviar(CMD_HOMING, 0);
    else if(strcmp(accion, "ir_led") == 0)              ok = experimento_enviar(CMD_IR_A_LED, valor);
    else if(strcmp(accion, "posicion_manual") == 0)     ok = experimento_enviar(CMD_POSICION_MANUAL, valor);
    else if(strcmp(accion, "pulso_hacia_home") == 0)    ok = experimento_enviar(CMD_PULSO_HACIA_HOME, valor);
    else if(strcmp(accion, "pulso_desde_home") == 0)    ok = experimento_enviar(CMD_PULSO_DESDE_HOME, valor);
    else if(strcmp(accion, "intensidad") == 0)          ok = experimento_enviar(CMD_LED_INTENSIDAD, valor);
    else if(strcmp(accion, "frenado") == 0)             ok = experimento_enviar(CMD_FRENADO, valor);
    else if(strcmp(accion, "curva") == 0)               ok = experimento_enviar(CMD_MEDIR_CURVA, 0);
    else if(strcmp(accion, "medir_led") == 0)           ok = experimento_enviar(CMD_MEDIR_LED, valor);
    else if(strcmp(accion, "experimento") == 0)         ok = experimento_enviar(CMD_EXPERIMENTO, 0);
    else {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Accion desconocida");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Comando web: %s (%ld)", accion, (long)valor);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"cola llena\"}");
}

/* GET /api/resultados: JSON enviado por partes para no usar un buffer grande */
static esp_err_t resultados_handler(httpd_req_t *req)
{
    char buf[256];
    experimento_obtener_resultados(&copia_resultados);
    httpd_resp_set_type(req, "application/json");

    httpd_resp_sendstr_chunk(req, "{\"curvas\":[");
    for(int i = 0; i < CANTIDAD_LEDS; i++) {
        curva_t *c = &copia_resultados.curvas[i];
        snprintf(buf, sizeof(buf),
                 "%s{\"lambda\":%d,\"f_thz\":%.2f,\"medida\":%s,\"oscura\":%.4f,\"corte\":\"%s\",\"u0\":%.4f,\"puntos\":[",
                 i > 0 ? "," : "", c->longitud_onda_nm, calculo_frecuencia_hz(c->longitud_onda_nm) / 1e12,
                 c->medida ? "true" : "false", c->corriente_oscura_na, texto_corte(c->corte), c->potencial_corte_v);
        httpd_resp_sendstr_chunk(req, buf);

        for(int k = 0; k < c->cantidad_puntos; k++) {
            snprintf(buf, sizeof(buf), "%s[%.4f,%.4f]", k > 0 ? "," : "",
                     c->puntos[k].frenado_v, c->puntos[k].corriente_na);
            httpd_resp_sendstr_chunk(req, buf);
        }
        httpd_resp_sendstr_chunk(req, "]}");
    }

    ajuste_planck_t *a = &copia_resultados.ajuste;
    snprintf(buf, sizeof(buf),
             "],\"ajuste\":{\"valido\":%s,\"n\":%d,\"h\":%.5e,\"u_h\":%.3e,\"w_ev\":%.4f,\"r2\":%.5f}}",
             copia_resultados.ajuste_valido ? "true" : "false", a->cantidad, a->h, a->u_h,
             a->funcion_trabajo_ev, a->r2);
    httpd_resp_sendstr_chunk(req, buf);
    return httpd_resp_sendstr_chunk(req, NULL);    // Fin de la respuesta
}

/* GET /api/resultados.csv */
static esp_err_t csv_handler(httpd_req_t *req)
{
    char buf[96];
    experimento_obtener_resultados(&copia_resultados);
    httpd_resp_set_type(req, "text/csv");
    httpd_resp_set_hdr(req, "Content-Disposition", "attachment; filename=\"curvas_iv.csv\"");

    httpd_resp_sendstr_chunk(req, "longitud_onda_nm,frenado_V,corriente_nA\n");
    for(int i = 0; i < CANTIDAD_LEDS; i++) {
        curva_t *c = &copia_resultados.curvas[i];
        for(int k = 0; k < c->cantidad_puntos; k++) {
            snprintf(buf, sizeof(buf), "%d,%.4f,%.4f\n", c->longitud_onda_nm,
                     c->puntos[k].frenado_v, c->puntos[k].corriente_na);
            httpd_resp_sendstr_chunk(req, buf);
        }
    }
    return httpd_resp_sendstr_chunk(req, NULL);
}

esp_err_t servidor_web_init(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 8;

    esp_err_t ret = httpd_start(&server, &config);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo iniciar el servidor HTTP");
        return ret;
    }

    httpd_uri_t rutas[] = {
        {.uri = "/",                    .method = HTTP_GET,  .handler = pagina_handler},
        {.uri = "/api/estado",          .method = HTTP_GET,  .handler = estado_handler},
        {.uri = "/api/comando",         .method = HTTP_POST, .handler = comando_handler},
        {.uri = "/api/resultados",      .method = HTTP_GET,  .handler = resultados_handler},
        {.uri = "/api/resultados.csv",  .method = HTTP_GET,  .handler = csv_handler},
    };
    for(int i = 0; i < sizeof(rutas) / sizeof(rutas[0]); i++) {
        httpd_register_uri_handler(server, &rutas[i]);
    }

    ESP_LOGI(TAG, "Servidor web iniciado en el puerto %d", config.server_port);
    return ESP_OK;
}
