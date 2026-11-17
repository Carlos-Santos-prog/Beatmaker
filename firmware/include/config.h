#pragma once
/*
 * config.h - Parametros ajustables del Beatmaker.
 * Todo lo que se calibra a mano vive aqui, no disperso en el codigo.
 */
#include <stdint.h>
#include "pines.h"

// ---------------- MIDI ----------------
constexpr uint8_t  CANAL_MIDI       = 1;
constexpr uint32_t DURACION_NOTA_MS = 25;   // noteOff automatico

// Pad 1..12 -> C2..B2 (MIDI 36..47), sin duplicados.
constexpr uint8_t NOTAS_MIDI[NUM_PADS] = {
  36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47
};

// ---------------- Deteccion de golpes ----------------
constexpr uint32_t PERIODO_MUESTREO_US = 25;   // 40 kHz total, ~3.3 kHz por pad
constexpr uint16_t UMBRAL_GOLPE   = 400;       // ADC 12 bits (0..4095)
constexpr uint16_t UMBRAL_REARME  = 100;       // debe bajar de aqui para aceptar otro golpe
constexpr uint32_t VENTANA_PICO_US = 2000;     // ventana para capturar el pico real
constexpr uint32_t BLOQUEO_PAD_US  = 20000;    // tiempo muerto minimo entre golpes del mismo pad

// Velocity: pico -> 1..127. Un piezo rara vez llega a 4095, por eso el
// "pico maximo" es configurable. GAMMA < 1 = mas sensible a golpes suaves.
constexpr uint16_t PICO_VELOCITY_MAX = 3000;   // pico que da velocity 127
constexpr float    VELOCITY_GAMMA    = 0.7f;

// Crosstalk: si otro pad golpeo hace menos de CROSSTALK_US con un pico mayor,
// un golpe nuevo cuyo pico sea < CROSSTALK_RATIO de ese se descarta.
constexpr uint32_t CROSSTALK_US    = 3000;
constexpr float    CROSSTALK_RATIO = 0.5f;

// ---------------- Hardware ADC ----------------
// Cada piezo ve 1M en serie: muestreo lento para cargar bien el capacitor del ADC.
// Poner a 0 para volver a muestreo rapido si se anade un buffer en el PCB.
#define PADS_MUESTREO_LENTO 1

// Prioridad de interrupciones (menor numero = mayor prioridad). El audio
// de la Audio Library debe quedar por encima.
constexpr uint8_t PRIORIDAD_ADC_TIMER = 160;

// ---------------- Depuracion ----------------
#define DEBUG_PADS 1   // imprime golpes por Serial (requiere USB_MIDI_SERIAL)

/*
 * Plan de controles (etapa 4, aun no implementado):
 *  Pots:    P1 volumen master | P2 BPM | P3 sensibilidad pads | P4 volumen de kit/filtro
 *  Botones: B1 Play/Stop | B2 Rec (grabar patron con pads) | B3 Shift | B4 Pattern/Kit
 *  Encoder: gira = navegar/valor, pulsa = confirmar
 */
