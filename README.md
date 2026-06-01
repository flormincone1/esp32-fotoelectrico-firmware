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

- Etapa actual: prueba PWM con LED externo.
- Objetivo: validar generacion PWM por LEDC antes de integrar el circuito analogico.
- El PWM sale por `GPIO18`.

## Salida esperada

Una vez cargado el firmware:

```text
Hola mundo desde ESP32-WROOM-32
Proyecto: digitalizacion del experimento de efecto fotoelectrico
PWM iniciado en GPIO 18, 1000 Hz, 10 bits
Inicio prueba PWM
Duty PWM: 0%
Duty PWM: 25%
Duty PWM: 50%
Duty PWM: 75%
Duty PWM: 100%
```

Conectar un LED externo en protoboard:

```text
GPIO18 -> resistencia 220/330 ohm -> anodo LED
catodo LED -> GND
```

El brillo debe cambiar siguiendo la secuencia de duty.
