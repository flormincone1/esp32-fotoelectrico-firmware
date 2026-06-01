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
Pendiente de medir/probar.
```

## Prueba 1b - PWM con LED externo

Objetivo: repetir la prueba PWM en protoboard luego de validar el LED onboard.

Conexion sugerida:

```text
GPIO18 -> resistencia 220/330 ohm -> LED -> GND
```

Resultado:

```text
Pendiente de medir/probar.
```

## Prueba 2 - Control de puente H simulado con LEDs

Objetivo: verificar las senales antes de conectar el motor real.

Conexion sugerida:

```text
GPIO19 -> resistencia 220/330 ohm -> LED DIR -> GND
GPIO21 -> resistencia 220/330 ohm -> LED PWM -> GND
```

Secuencia esperada:

```text
motor f 30
motor stop
motor r 30
motor stop
motor_test
```

Resultado:

```text
Pendiente de medir/probar.
```
