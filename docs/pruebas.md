# Pruebas funcionales

## Resumen

| Prueba | Resultado |
| --- | --- |
| Compilacion, flash y monitor serie | Validada |
| Hello world y LED onboard | Validada |
| PWM con LED onboard | Validada |
| PWM con LED externo | Validada en protoboard |
| PWM con motor y transistor | Validada en protoboard |
| Motor DC con L293D | Validada en hardware |
| PWM de LED + movimiento temporizado | Compila; prueba fisica pendiente |
| DAC interno en GPIO25 | Compila; medicion fisica pendiente |

## PWM

- **GPIO:** 2.
- **Frecuencia:** 500 Hz.
- **Resolucion:** 10 bits.

La secuencia de banco recorre el duty cycle de 0% a 100% y vuelve a 0%. Se verifico visualmente con el LED onboard y con un LED externo.

Conexion del LED externo:

```text
GPIO2 -> resistencia 220/330 ohm -> LED -> GND
```

## Motor con L293D

| L293D | ESP32 |
| --- | ---: |
| ENABLE 1,2 | GPIO21, PWM |
| INPUT 1 | GPIO19 |
| INPUT 2 | GPIO18 |

Condiciones validadas:

- Velocidad: 60%.
- Tiempo por sentido: 5 segundos.
- Pausa: 1 segundo.
- Resultado: giro adelante, giro atras y detencion correctos.
- GND del ESP32 y de la fuente del motor en comun.

## Integracion actual

La intensidad luminosa y el movimiento de la cremallera se controlan de forma independiente:

| Entrada | Accion |
| --- | --- |
| `+` | Aumenta el PWM del LED 5% |
| `-` | Disminuye el PWM del LED 5% |
| `ESPACIO` | Mueve el motor al 60% durante 5 segundos |
| `ESPACIO` siguiente | Repite el movimiento en sentido contrario |
| `S` | Detiene inmediatamente el motor |

Estado: compilacion correcta; validacion fisica pendiente.

## DAC interno

- Canal: `DAC_CHAN_0`.
- Salida: GPIO25.
- Rango digital: 0 a 255.
- Secuencia disponible: 0%, 25%, 50%, 75% y 100%.
- Estado: implementado y compilado; pendiente de medicion con multimetro u osciloscopio.
