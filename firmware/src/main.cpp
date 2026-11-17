#include <Arduino.h>
#include "pines.h"
#include "config.h"
#include "pads/pads.h"

static bool     notaActiva[NUM_PADS] = {};
static uint32_t inicioNotaMs[NUM_PADS] = {};

static void alGolpe(uint8_t pad, uint8_t velocity) {
  uint8_t nota = NOTAS_MIDI[pad];
  if (notaActiva[pad]) usbMIDI.sendNoteOff(nota, 0, CANAL_MIDI);
  usbMIDI.sendNoteOn(nota, velocity, CANAL_MIDI);
  notaActiva[pad] = true;
  inicioNotaMs[pad] = millis();
#if DEBUG_PADS
  Serial.printf("pad %u nota %u vel %u\n", pad + 1, nota, velocity);
#endif
}

void setup() {
  padsBegin(alGolpe);
}

void loop() {
  padsUpdate();

  // noteOff independiente por pad (permite acordes)
  uint32_t ahora = millis();
  for (uint8_t p = 0; p < NUM_PADS; p++) {
    if (notaActiva[p] && ahora - inicioNotaMs[p] >= DURACION_NOTA_MS) {
      usbMIDI.sendNoteOff(NOTAS_MIDI[p], 0, CANAL_MIDI);
      notaActiva[p] = false;
    }
  }

  while (usbMIDI.read()) {}   // vaciar entrada MIDI del host
}
