#pragma once
#include <stdint.h>

// Callback: se llama desde padsUpdate() (contexto loop, no ISR) por cada golpe.
// pad: 0..NUM_PADS-1, velocity: 1..127
typedef void (*PadHitCallback)(uint8_t pad, uint8_t velocity);

void padsBegin(PadHitCallback alGolpe);
void padsUpdate();                       // llamar en cada loop()

// Escala del umbral de golpe (0.5 = mas sensible, 2.0 = menos). Para el pot P3.
void padsSetSensibilidad(float escalaUmbral);

uint32_t padsOverflows();                // buffer circular desbordado (diagnostico)
uint32_t padsErroresADC();               // startSingleRead fallido (pin no valido en ADC0)
