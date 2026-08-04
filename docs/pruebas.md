# Pruebas funcionales

## Prueba 1 - PWM con LED onboard

Objetivo: verificar que el ESP32 genera PWM mediante LEDC.

Conexion:

```text
No requiere conexion externa.
PWM configurado sobre GPIO2, LED onboard.
```

Secuencia esperada:

```text
0% -> 25% -> 50% -> 75% -> 100% -> 75% -> 50% -> 25% -> 0%
```

Resultado:

```text
Validada como primera prueba de PWM.
```

## Prueba 1b - PWM con LED externo

Objetivo: repetir la prueba PWM en protoboard luego de validar el LED onboard.

Conexion probada:

```text
APP_PWM_GPIO -> resistencia 220/330 ohm -> LED -> GND
```

Resultado:

```text
Validada en protoboard. El LED cambio de brillo siguiendo el PWM.
```

## Prueba 1c - PWM con motor y transistor

Objetivo: verificar que la salida PWM puede comandar un motor DC mediante una etapa de transistor.

Conexion probada:

```text
APP_PWM_GPIO -> base del transistor
motor entre colector y VCC
diodo de freewheeling en paralelo con el motor
emisor del transistor -> GND
GND del ESP32 comun con la fuente del motor
```

Resultado:

```text
Validada en protoboard. El motor respondio al barrido de duty del PWM.
```

## Prueba 2 - Control de motor con L293D

Objetivo: verificar el control de velocidad, direccion y detencion de un motor DC mediante el L293D.

Conexion probada:

```text
GPIO21 -> ENABLE 1,2 del L293D (PWM)
GPIO19 -> INPUT 1 del L293D
GPIO18 -> INPUT 2 del L293D
Motor -> OUTPUT 1 y OUTPUT 2 del L293D
GND del ESP32 comun con GND de la fuente del motor
```

Configuracion usada:

```text
Velocidad: 60%
Giro por sentido: 5 segundos
Pausa entre movimientos: 1 segundo
```

Resultado:

```text
Validada en hardware. El motor giro en ambos sentidos y se detuvo correctamente.
```

## Integracion - PWM y direccion desde terminal

El modulo `motor_terminal_control.c` integra las senales usadas en la prueba L293D sin cambiar los GPIO.

Controles:

```text
+       aumenta la velocidad 5%
-       disminuye la velocidad 5%
ESPACIO alterna el sentido de giro
S       detiene el motor
```

Estado:

```text
Compila correctamente. Pendiente de validacion fisica con el motor.
```

## Etapa DAC interno

El modulo `dac_control.c` prepara el DAC interno del ESP32 en `DAC_CHAN_0` (GPIO25).
Permite establecer un valor digital de 0 a 255, un porcentaje de 0 a 100 y ejecutar una secuencia de prueba de 0%, 25%, 50%, 75% y 100%.

Estado:

```text
Codigo incorporado al proyecto. Pendiente de medir la salida fisica con multimetro u osciloscopio.
```
