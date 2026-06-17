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
