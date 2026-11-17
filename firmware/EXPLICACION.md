# Cómo funciona el firmware del Beatmaker

Guía para entender **todo** el código del Teensy 4.1: qué hace cada archivo, cómo viaja un golpe desde el piezo
hasta la nota MIDI, qué se cambió respecto al sketch original y qué falta por confirmar o explorar.

> Estado: el firmware actual cubre los **pads (piezos) → USB-MIDI**. Compila para Teensy 4.1, pero **no se ha probado en la
> placa**. El audio (DAC + microSD), los botones, pots, encoder, OLED, LEDs y el secuenciador **todavía no están escritos**
> (están planeados; ver la sección 9).

---

## 1. Mapa de archivos

```
firmware/
├── platformio.ini               Configuración de compilación (2 entornos)
├── include/
│   ├── pines.h                  Qué pin del Teensy va a cada cosa (sacado del PCB)
│   └── config.h                 Todos los valores ajustables (umbrales, notas, curva de velocity...)
├── src/
│   ├── main.cpp                 Programa principal: conecta los pads con USB-MIDI
│   └── pads/
│       ├── pads.h               Interfaz pública del módulo de pads
│       └── pads.cpp             El detector de golpes (ADC + buffer + lógica)
├── tools/dac_tone/dac_tone.cpp  Programa de prueba del DAC (seno de 440 Hz). No es el firmware principal
├── piezo_plotter/               SKETCH ORIGINAL del compañero (no se tocó)
└── README.md                    Guía general del firmware
```

Compilar el firmware principal: `pio run`. Compilar y cargar la prueba del DAC: `pio run -e test_dac -t upload`.

---

## 2. La idea general

Cada pad tiene un **piezoeléctrico** pegado debajo. Al golpearlo genera un pulso de voltaje que se amortigua en
pocos milisegundos (una "campanita" que se apaga). El firmware tiene que:

1. **Medir** ese voltaje muchas veces por segundo en los 12 piezos.
2. **Detectar** que hubo un golpe (cruzó un umbral).
3. **Medir su fuerza** (el pico más alto en una ventana de 2 ms) y convertirla en *velocity* MIDI (1–127).
4. **Enviar** la nota por USB-MIDI y apagarla poco después.

Todo esto debe ocurrir con latencia menor a 10 ms y sin perder golpes simultáneos, por eso el diseño separa
el trabajo en dos mundos:

| Mundo | Quién | Qué hace | Regla |
|---|---|---|---|
| **Interrupciones** | Timer + ISR del ADC | Medir y guardar muestras | Rápido y simple. Nada de lógica |
| **Bucle principal** | `loop()` | Analizar muestras, detectar golpes, enviar MIDI | Aquí vive toda la lógica |

El puente entre ambos es un **buffer circular** (sección 5).

---

## 3. `pines.h`: el mapa del hardware

Es la "tabla de conexiones" del PCB convertida en código. Nada más cambia de lugar un pin que este archivo.

| Función | Pines Teensy | Detalle |
|---|---|---|
| 12 piezos | 14, 15, 16, 17, 20, 21, 22, 23, 27, 26, 25, 24 | En orden de conectores J2…J13 |
| I2S (DAC) | 2 (DIN), 3 (LRCK), 4 (BCK), 33 (MCLK) | Es el bus **I2S2** |
| I2C (OLED) | 18 (SDA), 19 (SCL) | Conector J14 |
| LEDs WS2812 | 7 | Pasa por un 74LVC1G125 que sube la señal a 5 V |
| 4 botones | 6, 5, 36, 37 | **Activos en ALTO** (ver abajo) |
| Encoder | 29 (A), 28 (B), 30 (botón) | Pull-ups externos de 5.1 kΩ |
| 4 pots | 41, 40, 39, 38 | A17…A14 |
| Libres | 0, 1, 8–13, 31, 32, 34, 35 | |

### Detalles de pines que conviene saber

- **Los piezos se llaman por número de pin, no por `A0…A13`.** En el PCB, las redes (nombres de los cables en el
  esquemático) `A10…A13` están invertidas respecto a los nombres de pin del Teensy: el pin 24 (que Teensy llama `A10`)
  lleva la red `/A13` y va al conector J13. Si el código usara `A10…A13`, los pads 9–12 quedarían en orden inverso al
  de los conectores. Por eso `PINES_PAD` usa números y sigue el orden J2…J13.
- **Botones activos en alto:** cada pulsador conecta el pin a 3.3 V, y una resistencia de 10 kΩ a tierra lo mantiene
  en 0 cuando está suelto (más un condensador de 0.47 µF que hace de anti-rebote por hardware). Se leerán con
  `pinMode(pin, INPUT)` (sin pull-up) y "pulsado" es `HIGH`.
- **Pots:** el pot 1 está en el pin 41 = A17, el mismo pin que usaba el sketch original como "pad de prueba". En el PCB
  ese pin ya no es un pad.

---

## 4. `config.h`: los valores ajustables

Todo lo que se calibra a mano vive aquí. La regla del proyecto es no dispersar números mágicos por el código.

| Constante | Valor | Para qué sirve |
|---|---|---|
| `CANAL_MIDI` | 1 | Canal de salida |
| `DURACION_NOTA_MS` | 25 | Tras cuántos ms se manda el noteOff |
| `NOTAS_MIDI[12]` | 36…47 (C2…B2) | Una nota por pad, **sin duplicados** |
| `PERIODO_MUESTREO_US` | 25 | Cada 25 µs se inicia una conversión (40 kHz en total ≈ 3.3 kHz por pad) |
| `UMBRAL_GOLPE` | 400 | Valor ADC (0–4095) que debe superarse para considerar un golpe |
| `UMBRAL_REARME` | 100 | El pad debe bajar de aquí antes de aceptar otro golpe |
| `VENTANA_PICO_US` | 2000 | Durante 2 ms se busca el pico máximo |
| `BLOQUEO_PAD_US` | 20000 | Tiempo muerto mínimo entre dos golpes del mismo pad (20 ms) |
| `PICO_VELOCITY_MAX` | 3000 | Pico ADC que equivale a velocity 127 |
| `VELOCITY_GAMMA` | 0.7 | Forma de la curva (menor que 1 = más sensible a golpes suaves) |
| `CROSSTALK_US` / `CROSSTALK_RATIO` | 3000 / 0.5 | Reglas anti-vibración entre pads vecinos |
| `PADS_MUESTREO_LENTO` | 1 | Usa muestreo lento del ADC (explicado en la sección 6) |
| `PRIORIDAD_ADC_TIMER` | 160 | Prioridad de las interrupciones de medición |
| `DEBUG_PADS` | 1 | Imprime cada golpe por Serial |

Al final del archivo hay un comentario con el **plan de controles** (qué harían pots y botones). Ver sección 9.

---

## 5. `pads.cpp`: el detector de golpes (el corazón)

### 5.1 Cómo se miden los 12 piezos con un solo ADC

El Teensy tiene dos ADC, pero este código usa uno (`adc0`) en **round-robin**: un temporizador (`IntervalTimer`) se
dispara cada 25 µs y cada disparo mide el **siguiente** piezo de la lista. Tras 12 disparos (300 µs) vuelve al primero.

```
tick 0 → pad 1    tick 1 → pad 2   ...   tick 11 → pad 12   tick 12 → pad 1 ...
```

- `disparar()` (timer): anota qué canal se está convirtiendo y arranca la conversión con `startSingleRead`.
- `adcIsr()` (interrupción, se ejecuta cuando el ADC termina): lee el valor y lo guarda en el buffer junto con
  **el canal** y la **hora** (`micros()`) de esa medición. Guardar la hora en el momento exacto de la medición
  (y no cuando se procesa después) hace que las ventanas de tiempo sean precisas aunque `loop()` tarde.

### 5.2 El buffer circular

Es una lista de 1024 posiciones donde la ISR escribe y `loop()` lee, cada uno con su propio contador:

- `head`: lo mueve **solo** la ISR (dónde escribirá la próxima muestra).
- `tail`: lo mueve **solo** `loop()` (dónde leerá la próxima).

Como cada contador tiene un único dueño, no hacen falta bloqueos. Los contadores son `uint32_t` que crecen sin parar
y se convierten en posición con `& BUF_MASK` (por eso el tamaño debe ser potencia de 2). La diferencia `head - tail`
sigue siendo correcta incluso cuando el contador "da la vuelta" al llegar a 2³².

Si `loop()` se retrasa tanto que la ISR le saca más de 1024 muestras de ventaja, se descartan las más viejas y se
incrementa el contador `overflows` (diagnóstico).

### 5.3 La máquina de estados por pad

Cada pad tiene su propio estado, así que se pueden tocar **acordes** o varios pads casi a la vez. Para cada muestra
`procesar(canal, valor, t)` hace:

```
                 valor ≥ umbral y pad rearmado y pasó el bloqueo
   REPOSO ───────────────────────────────────────────────────────► CAPTURANDO
      ▲                                                                │ (cada muestra: ¿es el pico más alto?)
      │ valor < 100                                                    │ pasaron 2 ms
      │                                                                ▼
   ESPERANDO REARME ◄──────────────────────────────────────── FIN DE VENTANA → ¿crosstalk? → NOTA
```

1. **Rearme:** si el pad está "esperando rearme" y el valor bajó de `UMBRAL_REARME` (100), queda listo otra vez.
   Esto evita que la misma vibración dispare muchas notas.
2. **Inicio:** si no está capturando, está rearmado, pasó el tiempo de bloqueo (20 ms) y `valor ≥ umbral`, empieza
   la captura y se guarda la hora de inicio.
3. **Captura:** mientras tanto se actualiza el **pico** (el valor más alto visto).
4. **Fin de ventana:** a los 2 ms se cierra la captura. El pad pasa a "esperando rearme".
5. **Filtro de crosstalk:** si otro pad golpeó hace menos de 3 ms con un pico mayor y éste es menos de la mitad
   de ese pico, se considera vibración inducida por el vecino y se descarta.
6. **Nota:** se convierte el pico en velocity con la tabla (siguiente punto) y se llama al callback.

### 5.4 Curva de velocity (tabla precalculada)

Al iniciar se llena `lutVelocity[4096]`, una tabla que asocia cada valor de pico ADC con una velocity:

```
n = (pico − 400) / (3000 − 400)      limitado a 0…1
velocity = 1 + round( 126 × n^0.7 )  limitado a 1…127
```

Se usa una tabla en vez de calcular `pow()` en cada golpe para ahorrar tiempo. `PICO_VELOCITY_MAX = 3000` existe
porque un piezo rara vez llega a 4095; así un golpe fuerte normal ya alcanza velocity 127.

### 5.5 Funciones públicas (`pads.h`)

| Función | Qué hace |
|---|---|
| `padsBegin(callback)` | Configura pines (`INPUT_DISABLE`), ADC, interrupciones y arranca el temporizador |
| `padsUpdate()` | Se llama en cada `loop()`: vacía el buffer y ejecuta la máquina de estados |
| `padsSetSensibilidad(escala)` | Escala el umbral de golpe (0.25–4). Pensada para el pot P3. Menor = más sensible |
| `padsOverflows()` / `padsErroresADC()` | Contadores de diagnóstico |

`INPUT_DISABLE` apaga el "keeper" digital del pin (un circuito interno que mantiene el último nivel), que
falsearía las lecturas analógicas de alta impedancia.

---

## 6. Por qué se configura así el ADC (y la duda de la impedancia)

Cada piezo llega al ADC a través de una resistencia de **1 MΩ** (R4, R6… del PCB), con un clamp de diodos BAT54S a
tierra y a 3.3 V y una resistencia de 470 kΩ a tierra. La resistencia de 1 MΩ protege el pin, pero el ADC necesita una
fuente de baja impedancia para cargar su condensador interno de muestreo; con 1 MΩ las lecturas pueden salir **bajas
o contaminadas por el canal anterior** (el condensador no se descarga del todo entre medición y medición).

Por eso `PADS_MUESTREO_LENTO = 1` usa `LOW_SPEED` de muestreo y `MED_SPEED` de conversión (más tiempo para cargar
el condensador). El sketch original usaba `HIGH_SPEED` en ambos, lo opuesto de lo recomendable aquí.

---

## 7. `main.cpp`: pegamento entre pads y MIDI

Es corto a propósito:

- `setup()`: llama `padsBegin(alGolpe)`.
- `alGolpe(pad, velocity)` (callback): manda `usbMIDI.sendNoteOn`. Si la nota de ese pad aún estaba sonando, manda
  antes su `noteOff` para que el retrigger sea limpio. Guarda cuándo empezó (`millis()`).
- `loop()`: llama `padsUpdate()`, apaga las notas cuya duración (25 ms) terminó (cada pad con su propio contador,
  para permitir acordes) y ejecuta `while (usbMIDI.read()) {}` para **vaciar el MIDI entrante** (si el computador manda
  mensajes y no se leen, el buffer se llena y puede bloquear el puerto).

Con `DEBUG_PADS = 1` imprime `pad 3 nota 38 vel 87` por el puerto serie; sirve para calibrar sin DAW.
`platformio.ini` usa `USB_MIDI_SERIAL` (MIDI + puerto serie). Para el producto final se puede pasar a `USB_MIDI`.

---

## 8. Qué cambió respecto al sketch original (`piezo_plotter/…PruebaEscalable.ino`)

El sketch del compañero **no se modificó**. `src/` es una versión mejorada, separada en módulos:

| Tema | Sketch original | Versión nueva |
|---|---|---|
| Estructura | Todo en un archivo | `pines.h`, `config.h`, módulo `pads`, `main` |
| Velocity | `map(...,1,100)`: **máximo 100** aunque el resumen decía 127 | Tabla con curva ajustable, llega a 127 |
| Muestreo | 50 µs (≈ 1.7 kHz por pad, ~3 muestras en la ventana) | 25 µs (≈ 3.3 kHz, ~6 muestras) |
| Hora del pico | `micros()` al procesar (puede ir con retraso) | `micros()` guardado en la ISR |
| Impedancia | Muestreo rápido | Muestreo lento configurable |
| Notas | Índice 7 repetía F#2 (prueba temporal) | 36…47 sin repetir |
| Tiempo muerto | Solo rearme por umbral | Rearme + bloqueo de 20 ms |
| Crosstalk | No | Filtro entre pads |
| Prioridades | Sin definir | `NVIC_SET_PRIORITY` + prioridad del timer |
| MIDI entrante | No se leía | `while (usbMIDI.read())` |
| Errores ADC | No se comprobaban | Contador `erroresADC` |
| Pines de pads | `A0…A13` (orden invertido en 9–12 respecto a conectores) | Números de pin en orden de conectores |

---

## 9. Lo que falta (y cómo está pensado)

Nada de esto existe aún en código; son las etapas planeadas.

### Etapa 3: audio (DAC PCM5100A + microSD)
- Salida con `AudioOutputI2S2` (no `AudioOutputI2S`): los pines 2/3/4/33 son el bus I2S2.
- Reproductor de samples desde la microSD (slot nativo, `BUILTIN_SDCARD`), 8–16 voces.
- La librería de samples ya está en `../samples/` (7 kits de 12 WAV, 16 bits, 44.1 kHz, mono, nombres 8.3,
  pensados para `AudioPlaySdWav`). Los kits ocupan entre 340 KB y 1.1 MB cada uno.
- `tools/dac_tone` (entorno `test_dac`) es la primera prueba: un seno de 440 Hz para comprobar el DAC.

### Etapa 4: interfaz
- Botones, encoder, pots, OLED SSD1306 (I2C) y LEDs WS2812.
- Plan de controles propuesto (comentario en `config.h`): pots = volumen / BPM / sensibilidad de pads / volumen de
  kit o filtro; botones = Play-Stop / Rec / Shift / Pattern-Kit; el encoder navega y confirma.

### Etapa 5: secuenciador de 16 pasos
- Su temporización debería referenciarse al **contador de muestras de audio**, no a un timer de tiempo real,
  para evitar deriva entre patrones.

---

## 10. Cosas por confirmar o explorar (importante)

Ordenadas por riesgo. Muchas dependen de probar la placa real.

### Hardware (antes de fiarse del firmware)
1. **El pin XSMT del DAC está sin conectar.** El pin 17 del PCM5100A (soft mute) está marcado "sin conexión" en el
   esquemático. El datasheet dice que si no se usa debe ir a AVDD (3.3 V); flotante puede dejar la salida en
   silencio o en un estado impredecible. Es lo primero que hay que probar con `test_dac`. Si no suena, un cable
   desde el pin 17 a un punto de 3.3 V (por ejemplo el TP3) lo arregla, y en una v2 del PCB hay que conectarlo.
2. **Dos fuentes de 3.3 V en paralelo.** El regulador AMS1117 (U3) alimenta la red `+3.3V` y el pin 3.3 V del
   Teensy (que tiene su propio regulador) también está conectado a esa red. Conviene medir la red y ver que
   no hay problemas.
3. **Pin 7 y los LEDs:** `WS2812Serial` no admite el pin 7. Habría que usar `Adafruit_NeoPixel`/FastLED (que
   bloquean interrupciones mientras envían y podrían crear clics en el audio) u `OctoWS2811` con lista de pines propia.
   La **cantidad de LEDs** del strip aún no está definida.

### Firmware de pads
4. **¿Todos los pines de piezo están en el ADC0?** El código solo usa el ADC0 de la librería y no todos los pines
   analógicos están en ambos ADC. Si algún pad no responde, mirar `padsErroresADC()`. Si alguno falla, habría que
   repartir los pads entre ADC0 y ADC1 (lo cual además **doblaría** la frecuencia de muestreo por pad).
5. **El pico se subestima.** En el simulador (que usa un modelo supuesto de piezo), un golpe de intensidad 127 se leyó
   como velocity ≈ 108: con ~3.3 kHz por pad se pierde parte del primer pico. Hay que calibrar con golpes reales:
   `PICO_VELOCITY_MAX`, `VENTANA_PICO_US`, `VELOCITY_GAMMA` y quizá bajar `PERIODO_MUESTREO_US`.
6. **¿Se rearman los pads?** El rearme exige bajar de 100. Si por la resistencia de 470 kΩ, el ruido o el montaje el
   reposo queda por encima de 100, **un pad nunca se rearmaría**. Hay que ver el valor de reposo real (con
   `DEBUG_PADS` o un monitor) y ajustar `UMBRAL_REARME`.
7. **Prioridades de interrupción.** Timer y ADC están en 160. En el Teensy, número menor = prioridad mayor, y la
   Audio Library usa sus propias prioridades (interrupciones de DMA/I2S). Cuando exista el audio hay que comprobar
   que el escaneo no genere clics ni pierda golpes (`AudioProcessorUsageMax()` ayuda).
8. **Sensibilidad vs. curva.** `padsSetSensibilidad` cambia el umbral de detección, pero la tabla de velocity se
   construye con el `UMBRAL_GOLPE` fijo. Efecto: con el pot de sensibilidad se detectan más o menos golpes suaves, pero
   la curva de velocity no se desplaza. Es una decisión de diseño, pero conviene confirmar que es lo deseado.
9. **Latencia del audio con SD.** Leer samples directamente de la microSD puede añadir latencia o fallos si la
   tarjeta tarda. Para latencia menor a 10 ms hay que explorar precargar los samples del kit activo en RAM
   (el README ya lo propone). Los kits pesan entre 340 KB y 1.1 MB y la RAM libre ronda 500 KB (RAM2), así que quizá
   haga falta recortarlos, bajar a 8 bits o usar PSRAM externa.
10. **`usbMIDI.send_now()`.** Los mensajes MIDI se acumulan y salen cuando `loop()` termina (o por temporizador del
    core). Si se nota algo de retraso en el DAW, se puede forzar con `send_now()`.

### Proyecto
11. **Licencias de los samples.** Los kits TR-808 de Michael Fischer se describen como gratuitos sin restricciones, los
    kits LM-2 / RZ-1 / CR-8000 vienen de un repo que los llama "dominio público" (sin archivo LICENSE) y el kit DEMBOW
    usa el pack gratuito de MIDI Klowd. Es para uso interno/académico; no redistribuir (detalles en `samples/README.md`).
12. **El simulador** (`../simulador/beatmaker_sim.html`) es un port directo de `pads.cpp` y `main.cpp` a JavaScript
    con las mismas constantes, útil para probar la lógica sin la placa. Pero el piezo y el ADC allí son un
    modelo supuesto: **no sustituye** la calibración en el equipo real.

---

## 11. Resumen en una frase

Un temporizador mide los 12 piezos en rotación, una interrupción guarda cada medición con su hora, `loop()` busca el
pico de cada golpe en una ventana de 2 ms, lo convierte en velocity con una tabla, filtra rebotes y vibraciones vecinas
y manda la nota por USB-MIDI; todo lo demás (audio, interfaz, secuenciador) está diseñado pero pendiente.
