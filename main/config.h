#ifndef CONFIG_H
#define CONFIG_H

/*
 * Configuracion general del firmware.
 *
 * Todos los pines y parametros del experimento estan en este archivo.
 * Para adaptar el firmware al hardware solo deberia hacer falta tocar aca.
 */

#include "driver/gpio.h"
#include "driver/dac_types.h"
#include "hal/adc_types.h"

/* ------------------------------------------------------------------ */
/* Pines (ESP32 DevKit V1 de 30 pines)                                 */
/* ------------------------------------------------------------------ */

#define PIN_LED_PWM             GPIO_NUM_2      // J4 pin 4 (PWM_POT): etapa de potencia de los LED
#define PIN_MOTOR_PWM           GPIO_NUM_21     // J4 pin 11 (PWM): habilitacion del puente H
#define PIN_MOTOR_DIR           GPIO_NUM_19     // J4 pin 10 (DIR): sentido de giro
#define PIN_MOTOR_DIR_N         GPIO_NUM_18     // DIR invertido. Solo lo usa el L293D (IN2) del banco de pruebas
#define PIN_HOME                GPIO_NUM_15     // J4 pin 3: sensor optico de posicion inicial

#define DAC_CANAL_FRENADO       DAC_CHAN_0      // GPIO25 -> J2 pin 3 del amplificador (DAC_in)
#define ADC_CANAL_CORRIENTE     ADC_CHANNEL_4   // GPIO32 <- J2 pin 1 del amplificador (I_sens x 10)
#define ADC_CANAL_FRENADO       ADC_CHANNEL_5   // GPIO33 <- J2 pin 2 del amplificador (U0)

/* ------------------------------------------------------------------ */
/* Polaridad de las salidas                                            */
/* ------------------------------------------------------------------ */

/*
 * 0: placa con el segundo inversor (correccion C1). Duty 0 = apagado.
 * 1: placa original, donde un GPIO en bajo enciende la salida.
 *    Ojo: con 1 el arranque NO es seguro, porque durante el boot el pin queda en bajo.
 */
#define LED_PWM_INVERTIDO       0
#define MOTOR_PWM_INVERTIDO     0

/* Cambiar a 1 si la cremallera se mueve al reves de lo esperado */
#define MOTOR_DIR_INVERTIDO     0

/* Nivel que lee PIN_HOME cuando la cremallera esta en la posicion inicial */
#define HOME_NIVEL_ACTIVO       1

/* ------------------------------------------------------------------ */
/* LED                                                                 */
/* ------------------------------------------------------------------ */

#define LED_PWM_FRECUENCIA_HZ   5000
#define LED_INTENSIDAD_MEDICION 100     // [%] 100 % = luz continua, sin modulacion del PWM

/* ------------------------------------------------------------------ */
/* Motor y cremallera                                                  */
/* ------------------------------------------------------------------ */

#define MOTOR_PWM_FRECUENCIA_HZ 500
#define MOTOR_TIEMPO_MUERTO_MS  100     // Pausa con el motor apagado antes de cambiar el sentido

#define CREMALLERA_VELOCIDAD    60      // [%] velocidad de los movimientos
#define HOMING_TIMEOUT_MS       15000   // Si no encuentra el home en este tiempo, se detiene con error

/* Sentido de giro que acerca la cremallera al sensor de home */
#define SENTIDO_HACIA_HOME      MOTOR_ATRAS
#define SENTIDO_DESDE_HOME      MOTOR_ADELANTE

/* ------------------------------------------------------------------ */
/* Emisores                                                            */
/* ------------------------------------------------------------------ */

#define CANTIDAD_LEDS           5

/* Longitud de onda nominal de cada LED [nm], en el orden de la cremallera */
#define LEDS_LONGITUD_ONDA_NM   { 395, 470, 525, 625, 850 }

/*
 * Tiempo de movimiento desde el home hasta dejar cada LED frente a la fotocelula [ms].
 * VALORES A CALIBRAR: usar las teclas 'q' y 'w' del terminal y anotar los tiempos.
 */
#define LEDS_TIEMPO_DESDE_HOME_MS { 500, 1500, 2500, 3500, 4500 }

/* ------------------------------------------------------------------ */
/* Potencial de frenado (DAC) y medicion (ADC)                         */
/* ------------------------------------------------------------------ */

#define DAC_TENSION_MAXIMA_MV   3300    // Tension de salida del DAC con el valor 255

#define BARRIDO_FRENADO_MAX_MV  2500    // El ADC del ESP32 satura cerca de 2,5-3,1 V
#define BARRIDO_PASO_MV         25      // Paso del barrido (el DAC tiene ~13 mV por paso)
#define TIEMPO_ASENTAMIENTO_MS  200     // Espera despues de cada cambio antes de medir
#define MUESTRAS_POR_PUNTO      32      // Lecturas del ADC que se promedian en cada punto

/*
 * 1: el potencial de cada punto es el medido en U0 (GPIO33). Recomendado.
 * 0: se usa el valor pedido al DAC (sirve para probar sin el cable de U0).
 */
#define USAR_FRENADO_MEDIDO     1

/*
 * Ganancia del amplificador de corriente: V = I x 10 Mohm x 10.
 * 1 nA -> 100 mV.
 */
#define CORRIENTE_MV_POR_NA     100.0f

/* ------------------------------------------------------------------ */
/* Calculo del potencial de corte                                      */
/* ------------------------------------------------------------------ */

/*
 * El potencial de corte es la primera tension donde la fotocorriente
 * (ya sin la corriente oscura) cae por debajo de esta fraccion de la corriente sin frenado.
 */
#define UMBRAL_FRACCION_CORTE   0.02f

/* Si la corriente sin frenado es menor a esto, se considera que no hay fotoemision */
#define CORRIENTE_MINIMA_NA     0.05f

/* ------------------------------------------------------------------ */
/* Wi-Fi (punto de acceso propio, no necesita router)                  */
/* ------------------------------------------------------------------ */

#define WIFI_SSID               "Fotoelectrico-UTN"
#define WIFI_PASSWORD           "fotoelectrico"     // Minimo 8 caracteres
#define WIFI_CANAL              1
#define WIFI_MAX_CONEXIONES     4
/* La pagina queda en http://192.168.4.1 */

#endif /* CONFIG_H */
