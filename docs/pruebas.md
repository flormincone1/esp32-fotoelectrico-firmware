# Pruebas funcionales

## Prueba 1 - PWM con LED

Objetivo: verificar que el ESP32 genera PWM mediante LEDC.

Conexion sugerida:

```text
GPIO18 -> resistencia 220/330 ohm -> LED -> GND
```

Secuencia esperada:

```text
pwm 0
pwm 25
pwm 50
pwm 75
pwm 100
pwm_test
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
