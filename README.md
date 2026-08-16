# Firmware ESP32 - Efecto fotoelectrico

## Objetivo del proyecto

Modernizar el experimento de efecto fotoelectrico reemplazando ajustes y lecturas manuales por control y adquisicion automatizados. El ESP32 coordinara la intensidad de los LED, el posicionamiento de los emisores, el potencial de frenado y la medicion de la corriente fotoelectrica.

El objetivo final es ejecutar barridos reproducibles, reducir errores de operacion y presentar los resultados desde una interfaz web para analizar la relacion entre frecuencia, intensidad, corriente y potencial de corte.

## Arquitectura general

```text
Interfaz web -> Secuencia experimental -> PWM de LED
                                    |-> Motor y cremallera
                                    |-> DAC y potencial de frenado

Fotocelula -> Amplificador TIA -> ADC -> Procesamiento -> Interfaz web
```

## Etapas de desarrollo

| Etapa | Alcance | Estado |
| ---: | --- | --- |
| 1 | Proyecto ESP-IDF, compilacion, flash y monitor | Validada |
| 2 | Diagnostico inicial: hello world, contador y blinky | Validada; inactiva actualmente |
| 3 | Generacion PWM y pruebas de banco | Validada |
| 4 | Etapa final de potencia PWM y conjunto de cinco LED | Pendiente |
| 5 | Motor DC y puente H | Parcial; L293D validado |
| 6 | Control manual de luz y movimiento desde terminal | Compila; prueba fisica pendiente |
| 7 | Posicionamiento de cremallera, homing y limites | Pendiente |
| 8 | Generacion de tension con DAC interno | Compila; medicion pendiente |
| 9 | Adaptacion analogica del potencial de frenado | Pendiente |
| 10 | Amplificador de transimpedancia y adquisicion ADC | Pendiente |
| 11 | Maquina de estados y sincronizacion del experimento | Pendiente |
| 12 | Calculo, calibracion y almacenamiento de resultados | Pendiente |
| 13 | Comunicacion Wi-Fi e interfaz web | Pendiente |
| 14 | Protecciones y validacion integral con la fotocelula | Pendiente |

## Estado actual

**Fase:** cierre de las pruebas de actuadores y preparacion de la validacion DAC.

- Proyecto ESP-IDF, compilacion, flash y monitor serie validados.
- PWM validado con LED onboard, LED externo y carga de prueba; falta la etapa optica definitiva.
- Motor con L293D validado en ambos sentidos y con detencion.
- Integracion actual compilada: `+/-` regula la luz; `ESPACIO` mueve y alterna el motor; `S` lo detiene.
- Integracion PWM/motor corregida pendiente de validacion en hardware.
- DAC interno implementado en GPIO25, pero todavia inactivo y sin medicion fisica.
- ADC, posicionamiento absoluto, automatizacion experimental y web aun no iniciados.

Resultados de banco: [docs/pruebas.md](docs/pruebas.md).

## Proximos pasos - Roadmap

1. Flashear y validar la integracion independiente de PWM y movimiento temporizado.
2. Conectar el PWM a la etapa de potencia y comprobar la intensidad de los LED.
3. Definir posiciones, homing y limites de recorrido de la cremallera.
4. Medir la transferencia real del DAC y determinar rango, resolucion y error.
5. Integrar y calibrar la etapa analogica de potencial de frenado.
6. Implementar el ADC sobre ADC1 y validar la salida del amplificador TIA.
7. Crear la maquina de estados que sincronice LED, motor, DAC y ADC.
8. Implementar barridos, deteccion del potencial de corte y procesamiento fisico.
9. Desarrollar la interfaz Wi-Fi/web y el registro de resultados.
10. Agregar protecciones, manejo de errores y validacion integral del experimento.

## Tecnologias

- **Microcontrolador:** ESP32-WROOM-32.
- **Framework:** ESP-IDF 6.0.1 y FreeRTOS.
- **Lenguaje:** C.
- **Perifericos:** GPIO, LEDC/PWM, DAC y futuro ADC.
- **Hardware de prueba:** L293D, motor DC, LED y protoboard.
- **Herramientas:** Visual Studio Code, Git y GitHub.

## Pines actuales

| Funcion | GPIO | Estado |
| --- | ---: | --- |
| PWM de LED | 2 | Prueba de banco |
| Enable/PWM de motor | 21 | Validado con L293D |
| Direccion motor IN1 | 19 | Validado con L293D |
| Direccion motor IN2 | 18 | Validado con L293D |
| DAC interno, canal 0 | 25 | Pendiente de medicion |

> Los pines y rangos electricos son preliminares hasta validar el hardware final.

## Compilar y cargar

Desde una terminal ESP-IDF:

```powershell
idf.py set-target esp32
idf.py build
idf.py -p COM4 flash monitor
```
