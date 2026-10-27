#include "experimento.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "cremallera.h"
#include "frenado.h"
#include "led.h"
#include "medicion.h"
#include "motor.h"

static const char *TAG = "EXPERIMENTO";

#define LARGO_COLA          8
#define PERIODO_ESPERA_MS   10
#define PERIODO_LECTURA_MS  500     // Lectura de U e I mientras no hay tarea en curso

typedef struct {
    comando_t comando;
    int32_t valor;
} mensaje_t;

static QueueHandle_t cola_comandos;
static SemaphoreHandle_t mutex_datos;   // Protege 'estado' y 'resultados'
static volatile bool detener_pedido = false;

static estado_experimento_t estado;
static resultados_t resultados;
static curva_t curva_nueva;             // Curva en medicion (solo la usa la tarea)

static const uint16_t longitudes_onda_nm[CANTIDAD_LEDS] = LEDS_LONGITUD_ONDA_NM;

/* ------------------------------------------------------------------ */
/* Funciones auxiliares                                                */
/* ------------------------------------------------------------------ */

/* Cambia el estado y el mensaje que ve el usuario */
static void set_estado(estado_t nuevo, const char *formato, ...)
{
    xSemaphoreTake(mutex_datos, portMAX_DELAY);
    estado.estado = nuevo;
    va_list args;
    va_start(args, formato);
    vsnprintf(estado.mensaje, sizeof(estado.mensaje), formato, args);
    va_end(args);
    xSemaphoreGive(mutex_datos);

    if(nuevo == ESTADO_ERROR) {
        ESP_LOGE(TAG, "%s", estado.mensaje);
    }
    else {
        ESP_LOGI(TAG, "%s", estado.mensaje);
    }
}

/* Apaga todo lo que puede moverse o energizar la fotocelula */
static void salidas_seguras(void)
{
    motor_detener();
    led_apagar();
    frenado_set_mv(0);

    xSemaphoreTake(mutex_datos, portMAX_DELAY);
    estado.intensidad = 0;
    estado.frenado_mv = 0;
    xSemaphoreGive(mutex_datos);
}

/* Espera el tiempo pedido. Devuelve false si el usuario pidio detener */
static bool esperar_ms(uint32_t tiempo_ms)
{
    for(uint32_t t = 0; t < tiempo_ms; t += PERIODO_ESPERA_MS) {
        if(detener_pedido) {
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(PERIODO_ESPERA_MS));
    }
    return !detener_pedido;
}

static void set_intensidad(uint8_t porcentaje)
{
    led_set_intensidad(porcentaje);
    xSemaphoreTake(mutex_datos, portMAX_DELAY);
    estado.intensidad = porcentaje;
    xSemaphoreGive(mutex_datos);
}

static void set_frenado(uint16_t mv)
{
    frenado_set_mv(mv);
    xSemaphoreTake(mutex_datos, portMAX_DELAY);
    estado.frenado_mv = mv;
    xSemaphoreGive(mutex_datos);
}

/* Mide U e I y los guarda en el estado */
static void leer_entradas(float *frenado_v, float *corriente_na)
{
    float corriente = medicion_leer_corriente_na();
    float frenado = medicion_leer_frenado_v();

    xSemaphoreTake(mutex_datos, portMAX_DELAY);
    if(!USAR_FRENADO_MEDIDO) {
        frenado = estado.frenado_mv / 1000.0f;
    }
    estado.corriente_na = corriente;
    estado.frenado_v = frenado;
    estado.en_home = cremallera_en_home();
    xSemaphoreGive(mutex_datos);

    if(frenado_v != NULL) {
        *frenado_v = frenado;
    }
    if(corriente_na != NULL) {
        *corriente_na = corriente;
    }
}

static void set_led_actual(int led)
{
    xSemaphoreTake(mutex_datos, portMAX_DELAY);
    estado.led_actual = led;
    xSemaphoreGive(mutex_datos);
}

static int get_led_actual(void)
{
    xSemaphoreTake(mutex_datos, portMAX_DELAY);
    int led = estado.led_actual;
    xSemaphoreGive(mutex_datos);
    return led;
}

/* Recalcula h con todos los LED que tienen potencial de corte valido */
static void recalcular_ajuste(void)
{
    double frecuencias[CANTIDAD_LEDS];
    double cortes[CANTIDAD_LEDS];
    int cantidad = 0;

    xSemaphoreTake(mutex_datos, portMAX_DELAY);
    for(int i = 0; i < CANTIDAD_LEDS; i++) {
        curva_t *curva = &resultados.curvas[i];
        if(curva->medida && curva->corte == CORTE_OK) {
            frecuencias[cantidad] = calculo_frecuencia_hz(curva->longitud_onda_nm);
            cortes[cantidad] = curva->potencial_corte_v;
            cantidad++;
        }
    }
    resultados.ajuste_valido = calculo_constante_planck(frecuencias, cortes, cantidad, &resultados.ajuste);
    xSemaphoreGive(mutex_datos);

    if(resultados.ajuste_valido) {
        ESP_LOGI(TAG, "h = %.3e +- %.1e J s con %d LED (W = %.3f eV, R2 = %.4f)",
                 resultados.ajuste.h, resultados.ajuste.u_h, cantidad,
                 resultados.ajuste.funcion_trabajo_ev, resultados.ajuste.r2);
    }
}

/* ------------------------------------------------------------------ */
/* Tareas del experimento                                              */
/* ------------------------------------------------------------------ */

/* Mide la curva I-V del LED que esta frente a la fotocelula */
static esp_err_t medir_curva(int led)
{
    if(led < 0 || led >= CANTIDAD_LEDS) {
        set_estado(ESTADO_ERROR, "No se sabe que LED esta en posicion. Usar 1-5 o 'p'");
        return ESP_ERR_INVALID_STATE;
    }

    motor_detener();    // Nunca se mide con el motor en marcha
    memset(&curva_nueva, 0, sizeof(curva_nueva));
    curva_nueva.longitud_onda_nm = longitudes_onda_nm[led];

    // 1) Corriente oscura: LED apagado y sin frenado
    set_estado(ESTADO_MIDIENDO, "LED %d (%d nm): midiendo corriente oscura", led + 1, longitudes_onda_nm[led]);
    set_intensidad(0);
    set_frenado(0);
    if(!esperar_ms(5 * TIEMPO_ASENTAMIENTO_MS)) {
        return ESP_ERR_INVALID_STATE;
    }
    leer_entradas(NULL, &curva_nueva.corriente_oscura_na);

    // 2) Encender el LED y esperar que se estabilice
    set_intensidad(LED_INTENSIDAD_MEDICION);
    if(!esperar_ms(5 * TIEMPO_ASENTAMIENTO_MS)) {
        return ESP_ERR_INVALID_STATE;
    }

    // 3) Barrido del potencial de frenado
    set_estado(ESTADO_MIDIENDO, "LED %d (%d nm): barriendo el frenado", led + 1, longitudes_onda_nm[led]);
    int n = 0;
    for(int mv = 0; mv <= BARRIDO_FRENADO_MAX_MV && n < MAX_PUNTOS; mv += BARRIDO_PASO_MV) {
        set_frenado(mv);
        if(!esperar_ms(TIEMPO_ASENTAMIENTO_MS)) {
            return ESP_ERR_INVALID_STATE;
        }

        punto_t *punto = &curva_nueva.puntos[n];
        leer_entradas(&punto->frenado_v, &punto->corriente_na);
        n++;

        xSemaphoreTake(mutex_datos, portMAX_DELAY);
        estado.punto_actual = n;
        estado.puntos_totales = MAX_PUNTOS;
        xSemaphoreGive(mutex_datos);
    }
    curva_nueva.cantidad_puntos = n;
    curva_nueva.medida = true;

    // 4) Dejar todo apagado
    set_frenado(0);
    set_intensidad(0);

    // 5) Potencial de corte
    curva_nueva.corte = calculo_potencial_corte(curva_nueva.puntos, n, curva_nueva.corriente_oscura_na,
                                                UMBRAL_FRACCION_CORTE, CORRIENTE_MINIMA_NA,
                                                &curva_nueva.potencial_corte_v);

    xSemaphoreTake(mutex_datos, portMAX_DELAY);
    resultados.curvas[led] = curva_nueva;
    xSemaphoreGive(mutex_datos);

    if(curva_nueva.corte == CORTE_OK) {
        set_estado(ESTADO_ESPERANDO, "LED %d (%d nm): U0 = %.3f V", led + 1,
                   longitudes_onda_nm[led], curva_nueva.potencial_corte_v);
    }
    else if(curva_nueva.corte == CORTE_SIN_FOTOEMISION) {
        set_estado(ESTADO_ESPERANDO, "LED %d (%d nm): sin fotoemision", led + 1, longitudes_onda_nm[led]);
    }
    else {
        set_estado(ESTADO_ESPERANDO, "LED %d (%d nm): el corte supera %d mV", led + 1,
                   longitudes_onda_nm[led], BARRIDO_FRENADO_MAX_MV);
    }

    recalcular_ajuste();
    return ESP_OK;
}

static esp_err_t ir_a_led(int led)
{
    set_led_actual(-1);     // Mientras se mueve, la posicion es desconocida
    set_estado(ESTADO_MOVIENDO, "Moviendo al LED %d (%d nm)", led + 1, longitudes_onda_nm[led]);

    esp_err_t ret = cremallera_ir_a_led(led);
    if(ret == ESP_ERR_TIMEOUT) {
        set_estado(ESTADO_ERROR, "No se encontro el home. Revisar el sensor y el motor");
        return ret;
    }
    if(ret != ESP_OK) {
        return ret;
    }

    set_led_actual(led);
    set_estado(ESTADO_ESPERANDO, "LED %d (%d nm) en posicion", led + 1, longitudes_onda_nm[led]);
    return ESP_OK;
}

static esp_err_t experimento_completo(void)
{
    for(int led = 0; led < CANTIDAD_LEDS; led++) {
        esp_err_t ret = ir_a_led(led);
        if(ret != ESP_OK) {
            return ret;
        }
        ret = medir_curva(led);
        if(ret != ESP_OK) {
            return ret;
        }
    }

    if(resultados.ajuste_valido) {
        set_estado(ESTADO_ESPERANDO, "Experimento terminado: h = %.3e J s", resultados.ajuste.h);
    }
    else {
        set_estado(ESTADO_ESPERANDO, "Experimento terminado: no hay suficientes cortes para calcular h");
    }
    return ESP_OK;
}

static void ejecutar(mensaje_t *msg)
{
    esp_err_t ret = ESP_OK;

    switch(msg->comando) {
    case CMD_HOMING:
        set_led_actual(-1);
        set_estado(ESTADO_HOMING, "Buscando home");
        ret = cremallera_homing();
        if(ret == ESP_OK) {
            set_estado(ESTADO_ESPERANDO, "Cremallera en home");
        }
        else if(ret == ESP_ERR_TIMEOUT) {
            set_estado(ESTADO_ERROR, "No se encontro el home. Revisar el sensor y el motor");
        }
        break;

    case CMD_IR_A_LED:
        if(msg->valor < 0 || msg->valor >= CANTIDAD_LEDS) {
            set_estado(ESTADO_ERROR, "LED invalido: %ld", (long)msg->valor + 1);
            break;
        }
        ret = ir_a_led(msg->valor);
        break;

    case CMD_POSICION_MANUAL:
        if(msg->valor < 0 || msg->valor >= CANTIDAD_LEDS) {
            set_estado(ESTADO_ERROR, "LED invalido: %ld", (long)msg->valor + 1);
            break;
        }
        set_led_actual(msg->valor);
        set_estado(ESTADO_ESPERANDO, "Posicion manual: LED %ld (%d nm)",
                   (long)msg->valor + 1, longitudes_onda_nm[msg->valor]);
        break;

    case CMD_PULSO_HACIA_HOME:
    case CMD_PULSO_DESDE_HOME:
        set_led_actual(-1);
        cremallera_pulso(msg->comando == CMD_PULSO_HACIA_HOME ? SENTIDO_HACIA_HOME : SENTIDO_DESDE_HOME,
                         msg->valor);
        set_estado(ESTADO_ESPERANDO, "Pulso de %ld ms %s", (long)msg->valor,
                   msg->comando == CMD_PULSO_HACIA_HOME ? "hacia el home" : "desde el home");
        break;

    case CMD_LED_INTENSIDAD:
        set_intensidad(msg->valor < 0 ? 0 : (msg->valor > 100 ? 100 : msg->valor));
        set_estado(ESTADO_ESPERANDO, "Intensidad del LED: %ld %%", (long)msg->valor);
        break;

    case CMD_FRENADO:
        set_frenado(msg->valor < 0 ? 0 : (msg->valor > DAC_TENSION_MAXIMA_MV ? DAC_TENSION_MAXIMA_MV : msg->valor));
        set_estado(ESTADO_ESPERANDO, "DAC de frenado: %ld mV", (long)msg->valor);
        break;

    case CMD_MEDIR_CURVA:
        ret = medir_curva(get_led_actual());
        break;

    case CMD_MEDIR_LED:
        if(msg->valor < 0 || msg->valor >= CANTIDAD_LEDS) {
            set_estado(ESTADO_ERROR, "LED invalido: %ld", (long)msg->valor + 1);
            break;
        }
        ret = ir_a_led(msg->valor);
        if(ret == ESP_OK) {
            ret = medir_curva(msg->valor);
        }
        break;

    case CMD_EXPERIMENTO:
        ret = experimento_completo();
        break;
    }

    // Si algo fallo o se pidio detener, todo queda apagado
    if(ret != ESP_OK || detener_pedido) {
        salidas_seguras();
        if(detener_pedido) {
            set_estado(ESTADO_ESPERANDO, "Detenido por el usuario");
        }
    }
}

static void task_experimento(void *params)
{
    mensaje_t msg;

    while(1) {
        if(xQueueReceive(cola_comandos, &msg, pdMS_TO_TICKS(PERIODO_LECTURA_MS)) == pdTRUE) {
            detener_pedido = false;
            ejecutar(&msg);
        }
        else {
            // Sin comandos: se actualizan las lecturas para verlas en vivo
            leer_entradas(NULL, NULL);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Funciones publicas                                                  */
/* ------------------------------------------------------------------ */

esp_err_t experimento_init(void)
{
    cola_comandos = xQueueCreate(LARGO_COLA, sizeof(mensaje_t));
    mutex_datos = xSemaphoreCreateMutex();
    if(cola_comandos == NULL || mutex_datos == NULL) {
        return ESP_ERR_NO_MEM;
    }

    memset(&estado, 0, sizeof(estado));
    memset(&resultados, 0, sizeof(resultados));
    estado.led_actual = -1;
    for(int i = 0; i < CANTIDAD_LEDS; i++) {
        resultados.curvas[i].longitud_onda_nm = longitudes_onda_nm[i];
    }

    salidas_seguras();
    set_estado(ESTADO_ESPERANDO, "Listo");

    if(xTaskCreate(task_experimento, "task_experimento", 1024 * 6, NULL, 5, NULL) != pdPASS) {
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

bool experimento_enviar(comando_t comando, int32_t valor)
{
    mensaje_t msg = {.comando = comando, .valor = valor};
    return xQueueSendToBack(cola_comandos, &msg, 0) == pdTRUE;
}

void experimento_detener(void)
{
    detener_pedido = true;
    xQueueReset(cola_comandos);     // Descarta los comandos pendientes
    cremallera_cancelar();          // Corta el motor en el acto
    led_apagar();
    frenado_set_mv(0);

    xSemaphoreTake(mutex_datos, portMAX_DELAY);
    estado.intensidad = 0;
    estado.frenado_mv = 0;
    xSemaphoreGive(mutex_datos);
    ESP_LOGW(TAG, "DETENER pedido");
}

void experimento_obtener_estado(estado_experimento_t *copia)
{
    xSemaphoreTake(mutex_datos, portMAX_DELAY);
    *copia = estado;
    xSemaphoreGive(mutex_datos);
}

void experimento_obtener_resultados(resultados_t *copia)
{
    xSemaphoreTake(mutex_datos, portMAX_DELAY);
    *copia = resultados;
    xSemaphoreGive(mutex_datos);
}

const char *experimento_texto_estado(estado_t e)
{
    switch(e) {
    case ESTADO_ESPERANDO:  return "Esperando";
    case ESTADO_HOMING:     return "Homing";
    case ESTADO_MOVIENDO:   return "Moviendo";
    case ESTADO_MIDIENDO:   return "Midiendo";
    case ESTADO_ERROR:      return "Error";
    }
    return "?";
}
