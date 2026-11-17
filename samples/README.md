# Libreria de samples

Kits de bateria para la microSD del Beatmaker. Cada kit tiene 12 archivos (uno por pad),
en WAV 16-bit / 44.1 kHz / mono, el formato que reproduce `AudioPlaySdWav` del Teensy sin conversion.

## Estructura

```
samples/kits/
├── kits.json        # lista de kits y nombres de archivo (lo lee el simulador; el firmware lo usara)
├── KIT1/  808 CLASSIC
├── KIT2/  808 PUNCHY
└── KIT3/  808 DEEP
    └── 01_KICK.WAV ... 12_PERC.WAV
```

Para usarlos en el equipo, copiar las carpetas `KIT1`, `KIT2`, `KIT3` a la raiz de la microSD (FAT32).
Los nombres respetan el formato 8.3.

| Pad | Archivo | KIT1 | KIT2 | KIT3 |
|---|---|---|---|---|
| 1 | 01_KICK | BD2525 | BD7525 | BD0050 |
| 2 | 02_SNARE | SD5050 | SD7525 | SD2550 |
| 3 | 03_CLAP | CP | CP | CP |
| 4 | 04_HATC | CH | CH | CH |
| 5 | 05_HATO | OH50 | OH25 | OH75 |
| 6-8 | 06..08_TOM | LT/MT/HT 50 | LT/MT/HT 75 | LT/MT/HT 00 |
| 9 | 09_RIM | RS | RS | RS |
| 10 | 10_COWB | CB | CB | CB |
| 11 | 11_CRSH | CY5025 | CY2500 | CY7550 |
| 12 | 12_PERC | CL (claves) | MA (maracas) | LC25 (conga) |

La columna indica el archivo original del set de Michael Fischer (los numeros son la posicion de las perillas de la TR-808).

## Origen y licencia

Roland TR-808 Sound Sample Set 1.0.0 (1994) de **Michael Fischer** ("Technopolis"):
muestras grabadas directamente de una TR-808, 16-bit / 44.1 kHz. Su texto de presentacion las describe como
"absolutamente gratis" y el set se distribuye sin restricciones de licencia declaradas
(no es una dedicacion formal CC0). Obtenidas del paquete npm
[`@fluid-music/tr-808`](https://www.npmjs.com/package/@fluid-music/tr-808), que las toma de
[machines.hyperreal.org](http://machines.hyperreal.org/categories/drum-machines/TR-808/samples/).

Antes de un lanzamiento comercial conviene confirmar los terminos de uso con el autor o usar un set con licencia explicita.
