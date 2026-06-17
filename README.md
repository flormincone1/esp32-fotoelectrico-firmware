# Firmware ESP32 - Efecto fotoelectrico

Firmware base para el proyecto de digitalizacion del experimento de efecto fotoelectrico.

## Hardware inicial

- Microcontrolador: ESP32-WROOM-32.
- Framework: ESP-IDF.
- Control PWM: periferico LEDC.
- Prueba de motor actual: salida PWM hacia la base de un transistor, motor entre colector y VCC, con diodo de freewheeling.
- Control motor previsto: pin `DIR` + pin `PWM` hacia el puente H.

## Pines preliminares

| Senal | GPIO | Uso |
| --- | ---: | --- |
| PWM general | `APP_PWM_GPIO` | Prueba de PWM con LED y transistor |
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

- Etapa actual: PWM validado en protoboard.
- Resultado: el codigo PWM fue probado con un LED externo y tambien con un motor DC.
- Conexion de motor probada: `APP_PWM_GPIO` conectado a la base del transistor, motor entre colector y VCC, y diodo de freewheeling en paralelo con el motor.
- Objetivo siguiente: avanzar hacia el control de motor definitivo y validar el hardware final antes de integrar el puente H.

## Salida esperada

Una vez cargado el firmware:

```text
Hola mundo desde ESP32-WROOM-32
Proyecto: digitalizacion del experimento de efecto fotoelectrico
PWM iniciado en GPIO <APP_PWM_GPIO>, 500 Hz, 10 bits
```

El PWM realiza un barrido ascendente y descendente de duty cycle. Con un LED externo, el brillo debe variar progresivamente. Con el motor y el transistor, la velocidad debe cambiar siguiendo el duty aplicado.

Conexion de prueba con LED:

```text
APP_PWM_GPIO -> resistencia 220/330 ohm -> anodo LED
catodo LED -> GND
```

Conexion de prueba con motor:

```text
APP_PWM_GPIO -> base del transistor
motor entre colector y VCC
diodo de freewheeling en paralelo con el motor
emisor del transistor -> GND
GND del ESP32 comun con la fuente del motor
```
