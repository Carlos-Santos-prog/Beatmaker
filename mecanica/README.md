# 🛠️ Mecánica

Modelos 3D del chasis y los botones del beatmaker.

## Contenido

```
mecanica/
├── solidworks/    # Ensamblajes y piezas nativas (.SLDPRT, .SLDASM, .SLDDRW)
├── step/          # Exportaciones .STEP para intercambio CAD
└── stl/           # Exportaciones .STL para impresión 3D
```

## Carpetas

### `solidworks/`
Archivos nativos de SolidWorks: piezas, ensamblajes y planos. Es la fuente de verdad; los demás formatos se generan a partir de aquí.

### `step/`
Exportaciones en formato STEP para compartir con quien no use SolidWorks o para importar a otras herramientas (por ejemplo, para revisar el ajuste con la PCB de KiCad).

### `stl/`
Mallas para impresión 3D rápida de prototipos. Pueden no coincidir con la versión final si no se regeneran tras cambios.

## Piezas principales

- Chasis (base y tapa)
- Pads (matriz 4×4) y su alojamiento sobre los FSR
- Botones y perillas de encoders
- Ventana de la pantalla OLED
- Aberturas para USB, microSD y salida de audio

## Buenas prácticas

- Mantén los tres formatos sincronizados: exporta STEP y STL cada vez que cierres una versión del modelo.
- Usa nombres de archivo con versión, por ejemplo `chasis_v1.SLDPRT`.
- Verifica el ajuste con el modelo 3D de la PCB (exportado desde KiCad) antes de imprimir.
- Los archivos de SolidWorks son binarios y pesados: considera **Git LFS** para `.SLDPRT`, `.SLDASM` y `.STL`.

## Referencia en el cronograma

Diseño de carcasa en las semanas 8–9 (en paralelo a la PCB v1) y ensamblaje en carcasa definitiva en la semana 14.
