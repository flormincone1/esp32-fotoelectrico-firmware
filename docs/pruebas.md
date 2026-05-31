# Pruebas funcionales

## Prueba 1 - PWM con LED

Objetivo: verificar que el ESP32 genera PWM mediante LEDC.

Conexion sugerida:

```text
GPIO18 -> resistencia 220/330 ohm -> LED -> GND
```

Secuencia esperada:

```text
0% -> 25% -> 50% -> 75% -> 100% -> 75% -> 50% -> 25% -> 0%
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
Stop -> sentido A 30% -> stop -> sentido B 30% -> stop
```

Resultado:

```text
Pendiente de medir/probar.
```
