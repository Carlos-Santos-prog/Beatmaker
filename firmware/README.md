# 💻 Firmware

Código fuente en C/C++ para el **Teensy 4.1** del beatmaker: motor de audio, escaneo de pads, secuenciador, USB-MIDI e interfaz de usuario.

## Entorno de desarrollo

El proyecto se compila desde **Visual Studio Code** con **PlatformIO**, que gestiona el toolchain de Teensy sin tener que usar el Arduino IDE.

### Requisitos

- [Visual Studio Code](https://code.visualstudio.com/)
- Extensión **PlatformIO IDE**
- Cable USB y Teensy 4.1 (el uploader de PlatformIO usa Teensy Loader automáticamente)

### Configuración base (`platformio.ini`)

```ini
[env:teensy41]
platform = teensy
board = teensy41
framework = arduino
build_flags =
    -D USB_MIDI_SERIAL   ; USB-MIDI + Serial para depuración
    -D TEENSY_OPT_FASTER
monitor_speed = 115200
```

> Para producción puedes cambiar `USB_MIDI_SERIAL` por `USB_MIDI` y quitar el puerto serie.

### Compilar y cargar

1. Abre la carpeta `firmware/` en VS Code.
2. Compila con **PlatformIO: Build** (`Ctrl+Alt+B`).
3. Carga con **PlatformIO: Upload** (`Ctrl+Alt+U`) y pulsa el botón del Teensy si lo pide.

## Estructura prevista

```
firmware/
├── platformio.ini
├── src/
│   ├── main.cpp
│   ├── audio/        # Pipeline de audio, reproductor de samples, mezclador
│   ├── pads/         # Escaneo FSR, multiplexor, velocity curve, debounce
│   ├── sequencer/    # Step-sequencer de 16 pasos y patrones
│   ├── midi/         # USB-MIDI (usbMIDI) y MIDI DIN opcional
│   └── ui/           # OLED, encoders, LEDs RGB
├── include/
└── lib/
```

## Prioridades de tiempo real

Objetivo: latencia golpe → sonido **menor a 10 ms**.

1. **Máxima:** interrupción/DMA del códec de audio
2. **Alta:** escaneo de pads (~1 ms)
3. **Media:** generación y envío de MIDI
4. **Baja:** OLED y animación de LEDs

## Dependencias principales

- Teensyduino / `usbMIDI` (incluido en el core)
- [Teensy Audio Library](https://www.pjrc.com/teensy/td_libs_Audio.html)
- Librería para OLED SSD1306 y para LEDs WS2812B (por definir)

## Convenciones

- Un módulo por carpeta, con su `.h` y `.cpp`.
- Sin `delay()` bloqueante en el código de tiempo real.
- Los parámetros ajustables (curva de velocity, umbrales, debounce) van como constantes en un archivo de configuración, no dispersos en el código.

## Referencia

[Dirtywave M8 Firmware](https://github.com/Dirtywave): arquitectura de referencia sobre la misma familia de MCU.
