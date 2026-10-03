# 🥁 Beatmaker — Pad Controller / Drum Machine / Groovebox

Groovebox autónomo con **16 pads sensibles a la velocidad**, secuenciador de pasos y **salida USB-MIDI**, construido sobre **Teensy 4.1** y la **Teensy Audio Library**.

El dispositivo genera y reproduce su propio audio, sin depender de un DAW ni de una computadora para sonar. Opcionalmente, al conectarlo por USB, funciona también como controlador MIDI para un DAW.

> **Estado:** en desarrollo · Plan de ejecución: 4 meses / 5 personas

---

## Características

- 16 pads en matriz 4×4 con sensores FSR (velocity continua; aftertouch posible a futuro)
- Reproductor de samples polifónico (8–16 voces) desde microSD
- Step-sequencer de 16 pasos con patrones encadenables
- USB-MIDI class-compliant, sin drivers
- Pantalla OLED (BPM, patrón activo, kit), encoders rotativos y LEDs RGB por pad
- Salida de audio estéreo vía DAC I2S PCM5102
- MIDI DIN opcional, sujeto a tiempo disponible

**Fuera de alcance en v1:** captura de audio en vivo (looper con micrófono/línea) y síntesis compleja en tiempo real (posible fase 2 sin cambios de hardware).

---

## Hardware

| Subsistema | Componente / Solución |
|---|---|
| Microcontrolador | Teensy 4.1 (NXP iMXRT1062, Cortex-M7 @ 600 MHz, 1 MB RAM, microSD nativo, 2× I2S/TDM, 1× S/PDIF) |
| Sensado de pads | 16 FSR + multiplexor analógico 74HC4067 |
| Acondicionamiento | Filtro RC paso-bajo (~1–2 kHz) por canal |
| Audio | DAC PCM5102 (I2S, sin configuración I2C) |
| Pantalla | OLED SSD1306 (I2C) |
| Feedback visual | LEDs RGB direccionables WS2812B / SK6812 |
| Navegación | Encoders rotativos (interrupción de cuadratura) |
| Almacenamiento | microSD (bancos de samples / kits) |

**Decisiones de diseño**

- **Crosstalk:** debounce en firmware que ignora picos a menos de 2–3 ms de un golpe en un pad adyacente.
- **Velocity:** valor leído en una ventana de ~3–5 ms tras cruzar el umbral, mapeado a 0–127 con una tabla de lookup no lineal y ajustable.
- **Ruido de audio:** tierras analógica y digital separadas (star ground cerca del códec) y LDO de bajo ruido dedicado a la sección analógica.
- **LEDs:** controlados por un periférico dedicado (DMA/timer) para no interferir con el bus I2S.

---

## Firmware

### Prioridades de tiempo real

Objetivo: latencia golpe → sonido **menor a 10 ms**.

1. **Máxima:** interrupción/DMA del códec de audio (nunca se bloquea)
2. **Alta:** escaneo de pads (timer periódico ~1 ms)
3. **Media:** generación y envío de mensajes MIDI
4. **Baja:** refresco de OLED y animación de LEDs (los primeros en saltarse un ciclo bajo carga)

### Motor de sonido y secuenciador

- Bloques de la Teensy Audio Library: mezclador, reproductor de SD, control de ganancia
- Kit activo cargado en RAM, con prefetch anticipado desde SD para evitar cortes
- Temporización del secuenciador referenciada al **contador de muestras de audio**, no a un timer de tiempo real, para evitar deriva entre patrones

### Referencia

[Dirtywave M8](https://github.com/Dirtywave) (`M8Firmware` / `M8HeadlessFirmware`): groovebox basado en Teensy, útil como plantilla para el pipeline de audio y el secuenciador.

---

## Uso como controlador MIDI

Conectado por USB, el beatmaker se reconoce como dispositivo MIDI class-compliant mediante `usbMIDI` de Teensyduino, sin librerías adicionales. Esto permite grabar tus golpes en Ableton Live o tocar instrumentos de Kontakt.

Guía de configuración paso a paso: [`docs/teensy-usbmidi-ableton-kontakt.pdf`](docs/teensy-usbmidi-ableton-kontakt.pdf)

---

## Plan de ejecución (16 semanas)

| Semanas | Fase | Entregable |
|---|---|---|
| 1–2 | Especificación congelada, pedido de componentes | Documento de especificación |
| 3–5 | Prototipo en protoboard (1–2 pads, audio desde SD, primer MIDI USB) | Subsistemas por separado |
| 6–7 | Integración de 16 pads + códec + primer secuenciador | Prototipo funcional en mesa |
| 8–9 | Diseño y envío de PCB v1, diseño de carcasa | Gerbers + modelo 3D |
| 10–11 | Bring-up de PCB v1 | PCB validada |
| 12–13 | Firmware final: UI, velocity calibrada, kits, pruebas de estrés | Firmware *feature-complete* |
| 14 | Ensamblaje en carcasa definitiva | Unidades terminadas |
| 15 | Pruebas con músicos externos | Reporte de pruebas |
| 16 | Pulido, documentación y demo | Producto + documentación |

> Una falla de PCB no corregible por software en las semanas 10–11 obligaría a una v2 (+2–3 semanas).

### Roles

1. **Hardware / PCB:** esquemático, componentes, PCB, fabricación, pruebas eléctricas
2. **Firmware Audio/DSP:** pipeline de audio, samples, mezclador, SD, latencia
3. **Firmware Control/MIDI:** pads, velocity, encoders, secuenciador, USB-MIDI, UI OLED
4. **Integración mecánica / CAD 3D:** carcasa, ergonomía, ensamblaje
5. **Pruebas / Software host / Documentación:** plan de pruebas, herramienta para cargar kits, docs, cronograma

### Riesgos principales

| Riesgo | Mitigación |
|---|---|
| Crosstalk / velocity poco fiable | Validar el acondicionamiento con un solo pad en protoboard desde la semana 3 |
| Ruido digital en el audio | Definir tierras y alimentación analógica dedicada desde el primer esquemático |
| Bring-up hardware-firmware subestimado | Reservar semanas 10–11 solo para esto, sin features nuevas |

---

## Estructura del repositorio

```
.
├── firmware/        # Código Teensy (Arduino/PlatformIO)
├── hardware/        # Esquemáticos, PCB, Gerbers, BOM
├── cad/             # Modelos 3D de la carcasa
├── docs/            # Informe técnico y documentación de referencia
│   ├── informe_beatmaker.pdf
│   ├── teensy-usbmidi-ableton-kontakt.pdf
│   └── teensy41-datasheet.pdf   (clave.pdf)
├── samples/         # Kits de ejemplo para la microSD
└── README.md
```

---

## Referencias

- [Teensy 4.1 (PJRC)](https://www.pjrc.com/store/teensy41.html)
- [Teensy USB-MIDI](https://www.pjrc.com/teensy/td_midi.html)
- [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html)
- [Teensy Audio Library](https://www.pjrc.com/teensy/td_libs_Audio.html)
- [Dirtywave M8 Firmware](https://github.com/Dirtywave)

---

## Licencia

Por definir (por ejemplo, MIT para firmware y CERN-OHL para hardware).
