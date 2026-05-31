# Firmware ESP32 - Efecto fotoelectrico

Firmware base para el proyecto de digitalizacion del experimento de efecto fotoelectrico.

## Hardware inicial

- Microcontrolador: ESP32-WROOM-32.
- Framework: ESP-IDF.
- Control PWM: periferico LEDC.
- Control motor: pin `DIR` + pin `PWM` hacia el puente H.

## Pines preliminares

| Senal | GPIO | Uso |
| --- | ---: | --- |
| PWM general | 18 | Prueba de PWM con LED |
| DIR motor | 19 | Sentido del puente H |
| PWM motor | 21 | Velocidad/habilitacion del puente H |

> Nota: estos pines son provisorios. Antes de conectar el puente H real, validar niveles electricos y ajustar `main/app_config.h`.

## Compilar y cargar

Desde una terminal ESP-IDF:

```powershell
idf.py set-target esp32
idf.py build
idf.py flash monitor
```

## Estado

- Proyecto ESP-IDF base creado.
- Modulo PWM inicial.
- Modulo motor inicial.
- Secuencias de prueba por consola serial.
