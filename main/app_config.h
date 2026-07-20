#pragma once

// Pines preliminares para pruebas de banco. Ajustar antes de conectar hardware final.
#define APP_ONBOARD_LED_GPIO  2

// Salida PWM usada para pruebas con LED externo y etapa de transistor.
#define APP_PWM_GPIO        APP_ONBOARD_LED_GPIO
#define APP_MOTOR_DIR_GPIO  19
#define APP_MOTOR_PWM_GPIO  21

#define APP_PWM_FREQ_HZ     500
#define APP_PWM_RES_BITS    10

#define APP_TEST_DELAY_MS   0

#define APP_STATUS_DELAY_MS   5000

// Prueba aparte para modulo TB6612FNG, canal A.
#define TB6612_PWMA_GPIO       21
#define TB6612_AIN1_GPIO       19
#define TB6612_AIN2_GPIO       18
#define TB6612_STBY_GPIO       23

#define TB6612_PWM_FREQ_HZ     1000
#define TB6612_PWM_RES_BITS    10
#define TB6612_TEST_DELAY_MS   2000
