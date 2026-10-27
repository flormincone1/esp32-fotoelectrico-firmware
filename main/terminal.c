#include "terminal.h"

#include <stdbool.h>
#include <stdio.h>

#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "config.h"
#include "experimento.h"

static const char *TAG = "TERMINAL";

#define UART_TERMINAL       UART_NUM_0
#define PASO_INTENSIDAD     10      // [%]
#define PASO_FRENADO_MV     100
#define PULSO_MOTOR_MS      200

static resultados_t copia_resultados;   // static: es grande para la pila

static void mostrar_ayuda(void)
{
    printf("\n"
           "=========== Efecto fotoelectrico - comandos ===========\n"
           "  ?      esta ayuda\n"
           "  m      medir U e I una vez (y estado del sensor de home)\n"
           "  h      homing\n"
           "  1..5   mover la cremallera al LED 1..5\n"
           "  p + n  avisar que el LED n se coloco a mano (sin motor)\n"
           "  q / w  pulso de %d ms hacia / desde el home (calibrar)\n"
           "  + / -  intensidad del LED +/- %d %%\n"
           "  a / z  DAC de frenado +/- %d mV\n"
           "  c      medir la curva I-V del LED en posicion\n"
           "  e      experimento completo (todos los LED y h)\n"
           "  r      mostrar resultados\n"
           "  s      DETENER todo\n"
           "========================================================\n\n",
           PULSO_MOTOR_MS, PASO_INTENSIDAD, PASO_FRENADO_MV);
}

static void mostrar_medicion(void)
{
    estado_experimento_t e;
    experimento_obtener_estado(&e);
    printf("Estado: %s | %s\n", experimento_texto_estado(e.estado), e.mensaje);
    printf("  U = %.3f V | I = %.3f nA | LED %d al %d %% | DAC %d mV | home: %s\n",
           e.frenado_v, e.corriente_na, e.led_actual + 1, e.intensidad, e.frenado_mv,
           e.en_home ? "SI" : "no");
}

static void mostrar_resultados(void)
{
    experimento_obtener_resultados(&copia_resultados);

    printf("\nLED | lambda [nm] | f [THz] | I oscura [nA] | U0 [V]\n");
    for(int i = 0; i < CANTIDAD_LEDS; i++) {
        curva_t *c = &copia_resultados.curvas[i];
        printf(" %d  |    %4d     |  %5.1f  |", i + 1, c->longitud_onda_nm,
               calculo_frecuencia_hz(c->longitud_onda_nm) / 1e12);
        if(!c->medida) {
            printf("       -       | sin medir\n");
        }
        else if(c->corte == CORTE_OK) {
            printf("    %7.3f    | %.3f\n", c->corriente_oscura_na, c->potencial_corte_v);
        }
        else if(c->corte == CORTE_SIN_FOTOEMISION) {
            printf("    %7.3f    | sin fotoemision\n", c->corriente_oscura_na);
        }
        else {
            printf("    %7.3f    | fuera de rango\n", c->corriente_oscura_na);
        }
    }

    if(copia_resultados.ajuste_valido) {
        ajuste_planck_t *a = &copia_resultados.ajuste;
        printf("\nh = %.4e +- %.2e J s (%d LED) | error vs. referencia: %.1f %%\n",
               a->h, a->u_h, a->cantidad, 100.0 * (a->h - PLANCK_REFERENCIA) / PLANCK_REFERENCIA);
        printf("W = %.3f eV | R2 = %.4f\n", a->funcion_trabajo_ev, a->r2);
    }
    else {
        printf("\nh: hacen falta al menos 2 LED con potencial de corte\n");
    }

    // Curvas en CSV para copiar a una planilla
    printf("\nCSV\nlongitud_onda_nm,frenado_V,corriente_nA\n");
    for(int i = 0; i < CANTIDAD_LEDS; i++) {
        curva_t *c = &copia_resultados.curvas[i];
        for(int k = 0; k < c->cantidad_puntos; k++) {
            printf("%d,%.4f,%.4f\n", c->longitud_onda_nm, c->puntos[k].frenado_v, c->puntos[k].corriente_na);
        }
    }
    printf("FIN CSV\n\n");
}

/* Pide al experimento un nuevo valor de intensidad o frenado, a partir del actual */
static void cambiar_intensidad(int delta)
{
    estado_experimento_t e;
    experimento_obtener_estado(&e);
    int valor = e.intensidad + delta;
    experimento_enviar(CMD_LED_INTENSIDAD, valor < 0 ? 0 : (valor > 100 ? 100 : valor));
}

static void cambiar_frenado(int delta)
{
    estado_experimento_t e;
    experimento_obtener_estado(&e);
    int valor = e.frenado_mv + delta;
    experimento_enviar(CMD_FRENADO, valor < 0 ? 0 : (valor > DAC_TENSION_MAXIMA_MV ? DAC_TENSION_MAXIMA_MV : valor));
}

static void procesar_tecla(char tecla)
{
    static bool esperando_numero = false;     // Despues de 'p' se espera el numero de LED

    if(esperando_numero) {
        esperando_numero = false;
        if(tecla >= '1' && tecla < '1' + CANTIDAD_LEDS) {
            experimento_enviar(CMD_POSICION_MANUAL, tecla - '1');
        }
        else {
            printf("Numero de LED invalido\n");
        }
        return;
    }

    if(tecla >= '1' && tecla < '1' + CANTIDAD_LEDS) {
        experimento_enviar(CMD_IR_A_LED, tecla - '1');
        return;
    }

    switch(tecla) {
    case '?': mostrar_ayuda(); break;
    case 'm': mostrar_medicion(); break;
    case 'h': experimento_enviar(CMD_HOMING, 0); break;
    case 'p': esperando_numero = true; printf("LED colocado a mano (1-%d): ", CANTIDAD_LEDS); fflush(stdout); break;
    case 'q': experimento_enviar(CMD_PULSO_HACIA_HOME, PULSO_MOTOR_MS); break;
    case 'w': experimento_enviar(CMD_PULSO_DESDE_HOME, PULSO_MOTOR_MS); break;
    case '+': cambiar_intensidad(PASO_INTENSIDAD); break;
    case '-': cambiar_intensidad(-PASO_INTENSIDAD); break;
    case 'a': cambiar_frenado(PASO_FRENADO_MV); break;
    case 'z': cambiar_frenado(-PASO_FRENADO_MV); break;
    case 'c': experimento_enviar(CMD_MEDIR_CURVA, 0); break;
    case 'e': experimento_enviar(CMD_EXPERIMENTO, 0); break;
    case 'r': mostrar_resultados(); break;
    case 's':
    case 'S':
    case ' ': experimento_detener(); break;     // Espacio tambien detiene: es la tecla mas facil de encontrar
    default: break;
    }
}

static void task_terminal(void *params)
{
    uint8_t tecla;

    mostrar_ayuda();
    while(1) {
        // Espera hasta que llegue una tecla por el puerto serie
        if(uart_read_bytes(UART_TERMINAL, &tecla, 1, portMAX_DELAY) == 1) {
            procesar_tecla((char)tecla);
        }
    }
}

esp_err_t terminal_init(void)
{
    esp_err_t ret = uart_driver_install(UART_TERMINAL, 256, 0, 0, NULL, 0);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al instalar el driver de la UART");
        return ret;
    }

    if(xTaskCreate(task_terminal, "task_terminal", 1024 * 4, NULL, 3, NULL) != pdPASS) {
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}
