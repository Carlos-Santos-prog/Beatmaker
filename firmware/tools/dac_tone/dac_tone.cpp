/*
 * Prueba del DAC PCM5100A: seno de 440 Hz por I2S2 (pines 2/3/4/33), ambos canales.
 *
 * Si no se oye nada, medir el pin 17 (XSMT) del U4: debe estar a 3.3 V.
 * En el PCB v1 esta sin conectar; puentearlo a 3.3 V (ej. TP3) y repetir.
 *
 * Compilar/cargar:  pio run -e test_dac -t upload
 */
#include <Arduino.h>
#include <Audio.h>
#include "pines.h"

AudioSynthWaveformSine seno;
AudioOutputI2S2        salida;
AudioConnection        cableL(seno, 0, salida, 0);
AudioConnection        cableR(seno, 0, salida, 1);

void setup() {
  AudioMemory(10);
  seno.frequency(440.0f);
  seno.amplitude(0.2f);   // bajo, para no dañar oidos/altavoces
  Serial.println("Tono 440 Hz por I2S2 (DIN=2 LRCK=3 BCK=4 MCLK=33)");
}

void loop() {
  static uint32_t t = 0;
  if (millis() - t > 2000) {
    t = millis();
    Serial.printf("CPU audio: %.1f%%  mem max: %d\n",
                  AudioProcessorUsageMax(), AudioMemoryUsageMax());
  }
}
