#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "motor_control.h"
#include "pwm_control.h"

static const char *TAG = "app_main";

static void print_help(void)
{
    printf("\n");
    printf("Comandos disponibles:\n");
    printf("  help                 Muestra esta ayuda\n");
    printf("  pwm <0-100>          Ajusta duty del PWM de prueba\n");
    printf("  pwm_test             Ejecuta secuencia de prueba PWM\n");
    printf("  motor f <0-100>      Motor sentido forward con velocidad indicada\n");
    printf("  motor r <0-100>      Motor sentido reverse con velocidad indicada\n");
    printf("  motor stop           Detiene el motor\n");
    printf("  motor_test           Ejecuta secuencia de prueba del motor\n");
    printf("\n");
}

static uint8_t clamp_percent(int value)
{
    if (value < 0) {
        return 0;
    }
    if (value > 100) {
        return 100;
    }
    return (uint8_t)value;
}

static void handle_command(char *line)
{
    int value = 0;
    char direction = 0;

    if (strncmp(line, "help", 4) == 0) {
        print_help();
    } else if (sscanf(line, "pwm %d", &value) == 1) {
        ESP_ERROR_CHECK(pwm_control_set_duty_percent(clamp_percent(value)));
    } else if (strncmp(line, "pwm_test", 8) == 0) {
        pwm_control_test_sequence();
    } else if (sscanf(line, "motor %c %d", &direction, &value) == 2) {
        if (direction == 'f') {
            ESP_ERROR_CHECK(motor_control_set_direction(MOTOR_DIR_FORWARD));
            ESP_ERROR_CHECK(motor_control_set_speed_percent(clamp_percent(value)));
        } else if (direction == 'r') {
            ESP_ERROR_CHECK(motor_control_set_direction(MOTOR_DIR_REVERSE));
            ESP_ERROR_CHECK(motor_control_set_speed_percent(clamp_percent(value)));
        } else {
            ESP_LOGW(TAG, "Direccion invalida. Usar 'f' o 'r'.");
        }
    } else if (strncmp(line, "motor stop", 10) == 0) {
        ESP_ERROR_CHECK(motor_control_stop());
    } else if (strncmp(line, "motor_test", 10) == 0) {
        motor_control_test_sequence();
    } else {
        ESP_LOGW(TAG, "Comando desconocido: %s", line);
        print_help();
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Firmware efecto fotoelectrico - pruebas iniciales");

    ESP_ERROR_CHECK(pwm_control_init());
    ESP_ERROR_CHECK(motor_control_init());
    print_help();

    while (true) {
        char line[64] = {0};

        printf("> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] != '\0') {
            handle_command(line);
        }
    }
}
