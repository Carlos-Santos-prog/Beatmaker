# 📚 Documentación

Hojas de datos, especificaciones y documentos técnicos de referencia del proyecto.

## Documentos del proyecto

| Archivo | Descripción |
|---|---|
| `informe_beatmaker.pdf` | Arquitectura recomendada, hardware, firmware y plan de ejecución (4 meses / 5 personas) |
| `teensy-usbmidi-ableton-kontakt.pdf` | Guía para usar el Teensy 4.1 como controlador MIDI con Ableton Live y Kontakt |
| `teensy41-datasheet.pdf` | Ficha técnica y pinout del Teensy 4.1 |

## Hojas de datos de componentes clave

Componentes cuyos datasheets deben guardarse en esta carpeta:

| Componente | Documento |
|---|---|
| Procesador ARM Cortex-M7 (NXP i.MX RT1062, Teensy 4.1) | Manual de referencia del procesador |
| DAC **PCM5102** | Especificaciones y esquema de aplicación |
| Sensores **FSR** | Hoja de datos y esquemas de conexión |
| Multiplexor **74HC4067** | Hoja de datos |
| Pantalla **SSD1306** | Hoja de datos |
| LEDs **WS2812B / SK6812** | Hoja de datos |

## Organización sugerida

```
docs/
├── README.md
├── informe_beatmaker.pdf
├── teensy-usbmidi-ableton-kontakt.pdf
└── datasheets/
    ├── teensy41-datasheet.pdf
    ├── imxrt1062-reference-manual.pdf
    ├── pcm5102.pdf
    ├── fsr-connection-schematics.pdf
    ├── 74hc4067.pdf
    ├── ssd1306.pdf
    └── ws2812b.pdf
```

## Enlaces útiles

- [Teensy 4.1 (PJRC)](https://www.pjrc.com/store/teensy41.html)
- [Teensy USB-MIDI](https://www.pjrc.com/teensy/td_midi.html)
- [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html)
- [Teensy Audio Library](https://www.pjrc.com/teensy/td_libs_Audio.html)

## Notas

- Descarga los datasheets desde la página oficial de cada fabricante y conserva el nombre del componente en el archivo.
- Si actualizas un documento, reemplaza el archivo en lugar de duplicarlo con otro nombre.
