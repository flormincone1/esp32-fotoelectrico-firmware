# Firmware ESP32 - Efecto fotoelectrico

Firmware base para el proyecto de digitalizacion del experimento de efecto fotoelectrico.

## Hardware inicial

- Microcontrolador: ESP32-WROOM-32.
- Framework: ESP-IDF.
- Control PWM: periferico LEDC.
- Prueba de motor validada: motor DC controlado mediante L293D.
- Control integrado: velocidad PWM y direccion desde la terminal serie.
- Salida analogica: modulo para el DAC interno del ESP32 en GPIO25.

## Pines preliminares

| Senal | GPIO | Uso |
| --- | ---: | --- |
| PWM general | `APP_PWM_GPIO` | Prueba de PWM con LED y transistor |
| DIR motor | 19 | Sentido del puente H |
| PWM motor | 21 | Velocidad/habilitacion del puente H |
| IN2 motor | 18 | Segunda entrada de direccion del L293D |
| DAC | 25 | Salida analogica del DAC interno, canal 0 |

> Nota: estos pines son provisorios. Antes de conectar el puente H real, validar niveles electricos y ajustar `main/app_config.h`.

## Compilar y cargar

Desde una terminal ESP-IDF:

```powershell
idf.py set-target esp32
idf.py build
idf.py flash monitor
```

## Estado

- PWM validado con el LED onboard, un LED externo y un motor mediante transistor.
- Control de motor con L293D validado en ambos sentidos y con detencion.
- Integracion PWM + direccion por terminal compilada; validacion fisica pendiente.
- Modulo DAC interno implementado en GPIO25; medicion fisica pendiente.

Los resultados y conexiones de cada ensayo estan documentados en `docs/pruebas.md`.

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
