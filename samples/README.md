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

Para usarlos en el equipo, copiar las carpetas `KIT1` a `KIT7` a la raiz de la microSD (FAT32).
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

### Kits 4 a 6

Tomados de la coleccion [smpldsnds/drum-machines](https://github.com/smpldsnds/drum-machines),
descrita por su autor como "una coleccion de samples de dominio publico de distintas cajas de ritmos"
(el repo no incluye un archivo LICENSE; la declaracion esta solo en su README). Originales en OGG:
se convirtieron a WAV 16-bit / 44.1 kHz / mono, se recortaron las colas silenciosas, se limitaron a 2 s
y se normalizaron a -0.9 dBFS.

| Pad | KIT4 LM-2 | KIT5 RZ-1 | KIT6 CR-8000 |
|---|---|---|---|
| 1 KICK | kick | kick | kick |
| 2 SNARE | snare-m | snare | snare |
| 3 CLAP | clap | clap | clap |
| 4 HATC | hhclosed | hihat-closed | hihat-closed |
| 5 HATO | hhopen | hihat-open | hihat-open |
| 6 TOML | tom-l | tom-3 (el mas grave) | tom-low |
| 7 TOMM | tom-m | tom-2 | conga-low |
| 8 TOMH | tom-h | tom-1 (el mas agudo) | tom-high |
| 9 RIM | stick-m | clave | rimshot |
| 10 COWB | cowbell | cowbell | cowbell |
| 11 CRSH | crash | crash | cymball |
| 12 PERC | tambourine | ride | conga-high |

### Kit 7: DEMBOW

Hecho con one-shots de [MIDI Klowd, "Reggaeton One-Shots & Loops"](https://thebeatcommunity.com/2024/01/18/midi-klowd-release-reggaeton-one-shots-loops-a-free-download/)
(pack gratuito, anunciado como royalty-free; WAV 24-bit original). Uso interno del proyecto, **no redistribuir** el pack ni los WAV fuera del proyecto.
Importado con `tools/importar_kit.py` (WAV 16-bit / 44.1 kHz / mono, recortado a 2 s y normalizado). El pack original
no se versiona (`samples/_origen/` esta en `.gitignore`).

| Pad | Archivo original (carpeta del pack) |
|---|---|
| 1 KICK | Kicks / Kick_30 |
| 2 SNARE | Snares / Snare_12 |
| 3 CLAP | clap TR-808 (de KIT1; el pack no trae clap) |
| 4 HATC | Hi_Hats / Hi_Hat_8 |
| 5 HATO | Hats / Hat_4 |
| 6-8 TOM | Timbals / Timbal_6, 3 y 7 (de grave a agudo) |
| 9 RIM | Palitos / Palito_1 |
| 10 COWB | cowbell TR-808 (de KIT1; el pack no trae campana) |
| 11 CRSH | crash LM-2 (de KIT4; el pack no trae crash) |
| 12 PERC | Latin_Punchs / Latin_Punch_12 |

Para cambiar un sonido: `python samples/tools/importar_kit.py samples/_origen/klowd KIT7 --forzar 02_SNARE=Snares/ReggaetonLatino_Snare__20.wav`
(hay que repetir todos los `--forzar` de la tabla o el script elegira solo los slots no indicados).
