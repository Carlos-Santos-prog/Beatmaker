# 🔌 Hardware

Diseño electrónico del beatmaker en **KiCad**: esquemáticos, PCB y archivos de fabricación.

## Contenido

```
hardware/
├── beatmaker.kicad_pro        # Proyecto de KiCad
├── *.kicad_sch                # Esquemáticos jerárquicos
├── *.kicad_pcb                # Diseño de la PCB
├── lib/                       # Símbolos y huellas propias
├── gerbers/                   # Exportación para fabricación
└── bom/                       # Lista de materiales
```

## Esquemáticos jerárquicos

El diseño se divide en hojas por subsistema:

| Hoja | Descripción |
|---|---|
| **Raíz** | Interconexión general entre hojas |
| **Matriz de pads** | 16 FSR, filtro RC por canal (~1–2 kHz) y multiplexor 74HC4067 |
| **Códec de audio** | DAC PCM5102 (I2S) con aislamiento de ruido |
| **Alimentación** | Regulación, y LDO de bajo ruido dedicado a la sección analógica |
| **Controles y UI** | Encoders, OLED SSD1306 (I2C) y LEDs RGB WS2812B |
| **MCU** | Teensy 4.1, microSD y conexión USB |

## Aislamiento del códec

Para evitar ruido digital en el audio ("whining", clics):

- Planos de tierra analógica y digital separados, unidos en un único punto (**star ground**) cerca del códec.
- LDO de bajo ruido dedicado para la sección analógica, distinto del riel digital de 3.3 V.
- Definir esta separación desde el primer esquemático, no como corrección posterior.

## Fabricación

La carpeta `gerbers/` contiene la exportación lista para enviar a fábrica:

- Archivos Gerber de todas las capas
- Archivo de taladrado (Excellon)
- BOM y archivo de posiciones (si se pide ensamblaje)

> **Importante:** regenera siempre los Gerbers desde KiCad tras cualquier cambio en la PCB para que no queden desactualizados.

## Reglas de diseño

- Validar el circuito de acondicionamiento FSR con **un solo pad en protoboard** antes de integrar los 16.
- Revisar con ERC y DRC antes de cada exportación.
- El cronograma asume **una sola iteración de PCB**; un error no corregible por software en bring-up implica una v2 (+2–3 semanas).

## Requisitos

- [KiCad](https://www.kicad.org/) (usa la misma versión mayor en todo el equipo para evitar incompatibilidades).
