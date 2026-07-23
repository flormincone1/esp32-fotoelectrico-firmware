#pragma once

// Pines preliminares para pruebas de banco. Ajustar antes de conectar hardware final.
#define APP_ONBOARD_LED_GPIO  2

// Salida PWM usada para pruebas con LED externo y etapa de transistor.
#define APP_PWM_GPIO        APP_ONBOARD_LED_GPIO
#define APP_MOTOR_DIR_GPIO  19
#define APP_MOTOR_PWM_GPIO  21

#define APP_PWM_FREQ_HZ     500
#define APP_PWM_RES_BITS    10

#define APP_PWM_TEST_STEP_DELAY_MS  20

#define APP_MOTOR_TEST_SPEED_PERCENT  30
#define APP_MOTOR_TEST_RUN_MS          3000
#define APP_MOTOR_TEST_STOP_MS         1000

#define APP_STATUS_DELAY_MS   5000

// Prueba aparte para modulo TB6612FNG, canal A.
#define TB6612_PWMA_GPIO       21
#define TB6612_AIN1_GPIO       19
#define TB6612_AIN2_GPIO       18
#define TB6612_STBY_GPIO       23

#define TB6612_PWM_FREQ_HZ     1000
#define TB6612_PWM_RES_BITS    10
#define TB6612_TEST_DELAY_MS   2000

// Prueba aparte para L293D, usando el puente 1-2.
#define L293D_ENABLE_GPIO       21
#define L293D_IN1_GPIO          19
#define L293D_IN2_GPIO          18

#define L293D_PWM_FREQ_HZ       500
#define L293D_PWM_RES_BITS      10
#define L293D_TEST_SPEED_PERCENT  30
#define L293D_TEST_RUN_MS         3000
#define L293D_TEST_STOP_MS        1000
