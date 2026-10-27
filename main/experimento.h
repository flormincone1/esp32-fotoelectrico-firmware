#ifndef EXPERIMENTO_H
#define EXPERIMENTO_H

/*
 * Maquina de estados del experimento.
 *
 * Una sola tarea (task_experimento) maneja todo el hardware: LED, motor, DAC y ADC.
 * El terminal y la pagina web no tocan el hardware: le mandan comandos por una cola.
 */

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#include "calculo.h"
#include "config.h"

#define MAX_PUNTOS  (BARRIDO_FRENADO_MAX_MV / BARRIDO_PASO_MV + 1)

typedef enum {
    ESTADO_ESPERANDO = 0,   // Sin tarea en curso
    ESTADO_HOMING,          // Buscando la posicion inicial
    ESTADO_MOVIENDO,        // Llevando un LED frente a la fotocelula
    ESTADO_MIDIENDO,        // Barriendo el potencial de frenado
    ESTADO_ERROR,           // Algo fallo. Las salidas quedan apagadas
} estado_t;

typedef enum {
    CMD_HOMING = 0,         // valor: no se usa
    CMD_IR_A_LED,           // valor: indice del LED (0 a CANTIDAD_LEDS - 1)
    CMD_POSICION_MANUAL,    // valor: indice del LED que se coloco a mano
    CMD_PULSO_HACIA_HOME,   // valor: duracion [ms]
    CMD_PULSO_DESDE_HOME,   // valor: duracion [ms]
    CMD_LED_INTENSIDAD,     // valor: intensidad [%]
    CMD_FRENADO,            // valor: tension del DAC [mV]
    CMD_MEDIR_CURVA,        // valor: no se usa. Mide la curva del LED que esta en posicion
    CMD_MEDIR_LED,          // valor: indice del LED. Lo mueve y mide su curva
    CMD_EXPERIMENTO,        // valor: no se usa. Mide todos los LED y calcula h
} comando_t;

/* Estado que se muestra en el terminal y en la pagina web */
typedef struct {
    estado_t estado;
    char mensaje[80];           // Ultimo mensaje para el usuario
    int led_actual;             // Indice del LED en posicion, -1 si no se sabe
    uint8_t intensidad;         // [%]
    uint16_t frenado_mv;        // Tension pedida al DAC [mV]
    int punto_actual;           // Progreso del barrido
    int puntos_totales;
    float corriente_na;         // Ultima corriente medida [nA]
    float frenado_v;            // Ultimo potencial medido [V]
    bool en_home;
} estado_experimento_t;

/* Curva I-V de un LED */
typedef struct {
    uint16_t longitud_onda_nm;
    bool medida;
    int cantidad_puntos;
    punto_t puntos[MAX_PUNTOS];
    float corriente_oscura_na;  // Medida con el LED apagado
    resultado_corte_t corte;
    float potencial_corte_v;
} curva_t;

typedef struct {
    curva_t curvas[CANTIDAD_LEDS];
    bool ajuste_valido;
    ajuste_planck_t ajuste;
} resultados_t;

/**
 * @brief               Deja las salidas en estado seguro y crea la tarea del experimento
 * @retval ESP_OK       Inicializacion correcta
 */
esp_err_t experimento_init(void);

/**
 * @brief               Envia un comando a la tarea del experimento
 * @param[in] comando   Comando a ejecutar
 * @param[in] valor     Parametro del comando (ver comando_t)
 * @return bool         false si la cola esta llena
 */
bool experimento_enviar(comando_t comando, int32_t valor);

/**
 * @brief               Detiene todo: motor, LED y frenado. Se puede llamar desde cualquier tarea
 */
void experimento_detener(void);

/**
 * @brief               Copia el estado actual
 * @param[out] copia    Destino de la copia
 */
void experimento_obtener_estado(estado_experimento_t *copia);

/**
 * @brief               Copia los resultados (curvas y ajuste)
 * @param[out] copia    Destino de la copia (es grande: usar una variable static)
 */
void experimento_obtener_resultados(resultados_t *copia);

/**
 * @brief               Nombre legible de un estado
 */
const char *experimento_texto_estado(estado_t estado);

#endif /* EXPERIMENTO_H */
