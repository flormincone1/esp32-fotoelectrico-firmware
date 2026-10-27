# Pruebas funcionales

## Protocolo para probar el firmware final en el hardware

Hacer las pruebas en este orden: cada una usa lo que valido la anterior. Anotar en cada una la fecha, el commit probado y lo medido (fotos, capturas de osciloscopio, valores).

### 0. Antes de energizar

- [ ] La placa tiene las correcciones C1 a C4 del documento de hardware (o `LED_PWM_INVERTIDO` y `MOTOR_PWM_INVERTIDO` estan en 1, sabiendo que el arranque no es seguro).
- [ ] Con la placa sin alimentar, no hay continuidad entre GPIO19 y el colector de Q8.
- [ ] El pull-up del sensor de home va a 3,3 V.

### 1. Arranque seguro

1. Flashear y abrir `idf.py monitor`.
2. Durante el reset (boton EN) y el arranque, observar el motor y los LED.
3. **Esperado:** el motor no gira y los LED no encienden en ningun momento. El log termina con `Sistema listo`.
4. Desconectar el USB con la placa de 12 V alimentada: el motor y los LED tienen que quedar apagados.

### 2. LED

1. `+` varias veces: la intensidad sube de a 10 %.
2. Medir la tension de gate del IRF540N: 0 V con 0 %, ~11 V con 100 %.
3. Con 100 % el PWM queda en continua (sin pulsos en el osciloscopio).
4. `s`: los LED se apagan.

### 3. Motor y sentido

1. `w`: la cremallera se mueve 200 ms alejandose del home. `q`: se acerca.
2. Si el sentido esta al reves, cambiar `MOTOR_DIR_INVERTIDO` en `config.h`.
3. Mientras se mueve, `s` o `ESPACIO` lo detiene en el acto.
4. Medir la corriente del motor en marcha y bloqueado (1 s).

### 4. Sensor de home y homing

1. `m` con la cremallera lejos del home: `home: no`. Llevarla a mano al home: `home: SI`.
2. Si esta al reves, cambiar `HOME_NIVEL_ACTIVO`.
3. `h` desde distintas posiciones: siempre se detiene en el home.
4. Desconectar el sensor y hacer `h`: a los 15 s se detiene con error.

### 5. Calibrar la posicion de cada LED

1. `h`, despues `w` repetido hasta que el LED 1 quede frente a la fotocelula. Anotar la cantidad de pulsos (cada uno es de 200 ms).
2. Repetir para cada LED y cargar los tiempos en `LEDS_TIEMPO_DESDE_HOME_MS`.
3. Verificar con `1`..`5`. Repetir 10 veces un mismo LED y medir la dispersion de la posicion.

### 6. DAC de frenado

1. Con el amplificador desconectado, `a` de a 100 mV y medir GPIO25 con multimetro.
2. Anotar pedido vs. medido de 0 a 3,3 V: da el error y la alinealidad del DAC.
3. Conectar el amplificador (SW1 en DAC) y verificar que en la fotocelula aparece la tension invertida.

### 7. Medicion (ADC)

1. Con el amplificador conectado, `m` muestra U. Comparar con el multimetro en la salida U0.
2. Tapar la fotocelula: I tiene que quedar cerca de cero (corriente oscura).
3. Con un LED encendido y `a`/`z`, la corriente tiene que bajar al subir el frenado.
4. Verificar que las salidas del amplificador no superen 3,3 V ni bajen de 0 V en los pines del ESP32.

### 8. Primera curva I-U

1. Colocar un LED (si el motor todavia no esta listo, a mano y con `p` + numero).
2. `c`: mide la corriente oscura y despues barre de 0 a 2,5 V.
3. `r`: muestra U0 y las curvas en CSV.
4. Repetir 3 veces el mismo LED para ver la repetibilidad de U0.

### 9. Experimento completo y pagina web

1. Conectarse a la red `Fotoelectrico-UTN` y abrir http://192.168.4.1.
2. *Medir todos los LED y calcular h*.
3. **Esperado:** U0 crece con la frecuencia, el LED de 850 nm aparece "sin fotoemision" y h queda en el orden de 6,6e-34 J s.
4. *DETENER* en medio de un barrido: motor, LED y DAC quedan apagados.
5. *Descargar CSV* y comparar con lo que muestra la pagina.

## Prueba del calculo en la PC

`test/test_calculo.c` genera curvas sinteticas con h y W conocidos y verifica que el firmware los recupere. Resultado el 27-10-2026: todas las pruebas pasaron (h recuperada exacta, U0 con el sesgo esperado del criterio del 2 %).

## Historial: pruebas de banco con el firmware anterior

Estas pruebas se hicieron con la version anterior del firmware (hasta el commit `8f369b3`).

| Prueba | Resultado |
| --- | --- |
| Compilacion, flash y monitor serie | Validada |
| Hello world y LED onboard | Validada |
| PWM en GPIO2 (500 Hz, 10 bits) con LED onboard y LED externo | Validada en protoboard |
| PWM con motor y transistor | Validada en protoboard |
| Motor DC con L293D (ENABLE GPIO21, IN1 GPIO19, IN2 GPIO18), giro en ambos sentidos y detencion | Validada en hardware |
| PWM de LED + movimiento temporizado | Compilaba; no se probo |
| DAC interno en GPIO25 | Compilaba; no se midio |

El firmware nuevo sigue manejando el L293D del banco: GPIO21 a ENABLE, GPIO19 a IN1 y GPIO18 (sentido invertido) a IN2.
