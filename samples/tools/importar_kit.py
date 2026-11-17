"""
Importa un kit de samples cualquiera (WAV/MP3/OGG/FLAC...) y lo deja listo para el Beatmaker:
12 archivos 01_KICK.WAV ... 12_PERC.WAV en WAV 16-bit / 44.1 kHz / mono.

Clasifica por palabras clave del nombre del archivo (en espanol e ingles).

Uso (desde la raiz del repo):
  python samples/tools/importar_kit.py <carpeta_origen> KIT8
  python samples/tools/importar_kit.py <carpeta_origen> KIT8 --listar            (solo muestra la clasificacion)
  python samples/tools/importar_kit.py <carpeta_origen> KIT8 --forzar 02_SNARE=mi_caja.wav 09_RIM=palito2.wav

Requiere ffmpeg en el PATH, numpy y scipy.
"""
import argparse
import os
import re
import subprocess
import tempfile
import numpy as np
from scipy.io import wavfile

SR = 44100
MAX_SEG = 2.0           # segundos maximos por sample
EXT = ('.wav', '.mp3', '.ogg', '.flac', '.aif', '.aiff', '.m4a')

# slot -> palabras clave (se busca en el nombre en minusculas, sin acentos)
SLOTS = [
    ('01_KICK',  ['kick', 'bombo', 'bd', 'bassdrum', 'bass drum']),
    ('02_SNARE', ['snare', 'caja', 'sd', 'snr']),
    ('03_CLAP',  ['clap', 'palma']),
    ('04_HATC',  ['closed', 'cerrado', 'chh', 'chat', 'hat_c', 'hatc', 'hihat', 'hi-hat', 'hat']),
    ('05_HATO',  ['open', 'abierto', 'ohh', 'ohat', 'hat_o', 'hato']),
    ('06_TOML',  ['tom', 'conga', 'bongo']),       # los 3 toms se ordenan por tono
    ('07_TOMM',  []),
    ('08_TOMH',  []),
    ('09_RIM',   ['rim', 'palito', 'stick', 'sidestick', 'click', 'wood']),
    ('10_COWB',  ['cowbell', 'cowb', 'campana', 'bell']),
    ('11_CRSH',  ['crash', 'crsh', 'cymbal', 'platillo', 'ride']),
    ('12_PERC',  ['timbal', 'timbale', 'paila', 'perc', 'shaker', 'guira', 'gueiro', 'tamb', 'clave', 'fx']),
]
OPEN_WORDS = ['open', 'abierto', 'ohh', 'ohat', 'hat_o', 'hato']


def norm(s):
    s = s.lower()
    for a, b in zip('áéíóúñü', 'aeiounu'):
        s = s.replace(a, b)
    return s


def listar(origen):
    out = []
    for r, _, fs in os.walk(origen):
        for f in fs:
            if f.lower().endswith(EXT):
                out.append(os.path.join(r, f))
    return sorted(out)


def cargar(path):
    """Convierte con ffmpeg a mono 44.1k int16 y devuelve float32 en [-1,1]."""
    tmp = tempfile.mktemp(suffix='.wav')
    subprocess.run(['ffmpeg', '-y', '-loglevel', 'error', '-i', path, '-ac', '1', '-ar', str(SR),
                    '-sample_fmt', 's16', tmp], check=True)
    _, x = wavfile.read(tmp)
    os.remove(tmp)
    return x.astype(np.float32) / 32768.0


def freq_dom(x):
    seg = x[:SR // 2]
    sp = np.abs(np.fft.rfft(seg * np.hanning(len(seg))))
    sp[:5] = 0
    return np.fft.rfftfreq(len(seg), 1 / SR)[sp.argmax()]


def procesar(x):
    idx = np.where(np.abs(x) > 10 ** (-60 / 20))[0]
    if len(idx):
        x = x[:idx[-1] + 1]                       # recorta cola silenciosa
    mx = int(SR * MAX_SEG)
    if len(x) > mx:
        x = x[:mx].copy()
        f = int(SR * 0.05)
        x[-f:] *= np.linspace(1, 0, f)
    pk = np.abs(x).max()
    if pk > 0:
        x = x * (0.9 / pk)
    return (x * 32767).astype(np.int16)


def clasificar(archivos):
    cand = {s: [] for s, _ in SLOTS}
    for p in archivos:
        n = norm(os.path.basename(p))
        es_open = any(w in n for w in OPEN_WORDS)
        for slot, kws in SLOTS:
            if slot in ('07_TOMM', '08_TOMH'):
                continue
            if slot == '04_HATC' and es_open:
                continue
            if slot == '06_TOML' and not any(k in n for k in kws):
                continue
            if any(k in n for k in kws):
                cand[slot].append(p)
                break
    return cand


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('origen'); ap.add_argument('kit')
    ap.add_argument('--listar', action='store_true')
    ap.add_argument('--forzar', nargs='*', default=[], help='SLOT=archivo (relativo al origen o ruta)')
    a = ap.parse_args()

    archivos = listar(a.origen)
    cand = clasificar(archivos)
    elegido = {}
    for s, _ in SLOTS:
        if s in ('06_TOML', '07_TOMM', '08_TOMH'):
            continue
        if cand[s]:
            elegido[s] = cand[s][0]
    toms = cand['06_TOML'][:6]
    for f in a.forzar:
        slot, nombre = f.split('=', 1)
        ruta = nombre if os.path.exists(nombre) else os.path.join(a.origen, nombre)
        if not os.path.exists(ruta):
            m = [p for p in archivos if norm(nombre) in norm(os.path.basename(p))]
            ruta = m[0] if m else None
        if ruta: elegido[slot] = ruta

    datos = {}
    if toms:
        t = sorted(toms, key=lambda p: freq_dom(cargar(p)))
        if len(t) >= 3:   # grave, medio, agudo
            t = [t[0], t[len(t) // 2], t[-1]]
        for s, p in zip(('06_TOML', '07_TOMM', '08_TOMH'), t):
            elegido.setdefault(s, p)

    print(f'{len(archivos)} archivos encontrados')
    for s, _ in SLOTS:
        print(f'  {s:9} <- {os.path.relpath(elegido[s], a.origen) if s in elegido else "(FALTA)"}'
              + (f'   [{len(cand[s])-1} alternativas]' if len(cand[s]) > 1 else ''))
    if a.listar:
        return
    destino = os.path.join(os.path.dirname(__file__), '..', 'kits', a.kit)
    os.makedirs(destino, exist_ok=True)
    for s, p in elegido.items():
        wavfile.write(os.path.join(destino, s + '.WAV'), SR, procesar(cargar(p)))
    faltan = [s for s, _ in SLOTS if s not in elegido]
    print('Kit escrito en', os.path.normpath(destino))
    if faltan:
        print('Faltan slots (usa --forzar SLOT=archivo):', ', '.join(faltan))


if __name__ == '__main__':
    main()
