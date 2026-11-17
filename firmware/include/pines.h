#pragma once
/*
 * pines.h - Mapa de pines del Beatmaker (Teensy 4.1), derivado de
 * hardware/Indutrial/Indutrial.kicad_pcb.
 * Fuente de verdad: el PCB. Si cambia el PCB, actualizar aqui.
 */
#include <stdint.h>

// ---------------- Piezos (12 pads) ----------------
// Orden = conectores J2..J13 del PCB (pad 1..12).
// OJO: las redes /A10../A13 del PCB estan invertidas respecto al nombre del
// pin Teensy: la red /A10 esta en el pin 27 (A13 de Teensy), y la red /A13 en
// el pin 24 (A10 de Teensy). Por eso se usan numeros de pin, no macros A#.
constexpr uint8_t NUM_PADS = 12;
constexpr uint8_t PINES_PAD[NUM_PADS] = {
  14, // J2  red /A0
  15, // J3  red /A1
  16, // J4  red /A2
  17, // J5  red /A3
  20, // J6  red /A6
  21, // J7  red /A7
  22, // J8  red /A8
  23, // J9  red /A9
  27, // J10 red /A10
  26, // J11 red /A11
  25, // J12 red /A12
  24  // J13 red /A13
};
// Front-end por pad: clamp BAT54S (GND/3.3V), 470k a GND, 1M serie al ADC.

// ---------------- I2S -> DAC PCM5100A (bus I2S2) ----------------
// Usar AudioOutputI2S2 (no AudioOutputI2S).
constexpr uint8_t PIN_I2S_DIN  = 2;   // OUT2
constexpr uint8_t PIN_I2S_LRCK = 3;   // LRCLK2
constexpr uint8_t PIN_I2S_BCK  = 4;   // BCLK2
constexpr uint8_t PIN_I2S_MCLK = 33;  // MCLK2 -> SCK del DAC
// XSMT (pin 17 del PCM5100A) esta sin conectar en el PCB: verificar mute.

// ---------------- I2C (OLED, conector J14) ----------------
constexpr uint8_t PIN_SDA = 18;  // Wire
constexpr uint8_t PIN_SCL = 19;  // Wire (pull-ups 5.1k externos)

// ---------------- LEDs WS2812 (J16, via 74LVC1G125 a 5 V) ----------------
// Pin 7 no es compatible con WS2812Serial: usar NeoPixel/FastLED/OctoWS2811.
constexpr uint8_t PIN_LEDS = 7;

// ---------------- Botones (activos en ALTO) ----------------
// Pulsador a 3.3V, pulldown 10k + 0.47uF: pinMode(INPUT), pulsado = HIGH.
constexpr uint8_t NUM_BOTONES = 4;
constexpr uint8_t PINES_BOTON[NUM_BOTONES] = { 6, 5, 36, 37 };  // B1..B4

// ---------------- Encoder rotatorio ----------------
// Pull-ups externos 5.1k a 3.3V; comun a GND.
constexpr uint8_t PIN_ENC_A   = 29;  // red Encoder_pin_2
constexpr uint8_t PIN_ENC_B   = 28;  // red Encoder_pin_1
constexpr uint8_t PIN_ENC_BTN = 30;  // activo en ALTO (pulldown 10k)

// ---------------- Potenciometros (100k, 0..3.3V) ----------------
constexpr uint8_t NUM_POTS = 4;
constexpr uint8_t PINES_POT[NUM_POTS] = { 41, 40, 39, 38 };  // P1..P4 (A17..A14)

// ---------------- Pines libres ----------------
// 0, 1, 8-13, 31, 32, 34, 35. microSD: slot nativo (BUILTIN_SDCARD).
