/*
 * Teensy 4.1 - Matriz de piezos a USB MIDI con ADC por interrupcion.
 *
 * MODO_PRUEBA_A17 = 1: prueba un solo piezo en A17, que envia C2.
 * MODO_PRUEBA_A17 = 0: usa 12 piezos; A9 se asigna temporalmente a F#2.
 * A4 y A5 quedan libres para I2C.
 */
#include <ADC.h>

ADC *adc = new ADC();
IntervalTimer timerMuestreo;

// ---------------- Seleccion de cableado ----------------
#define MODO_PRUEBA_A17 0

#if MODO_PRUEBA_A17
const uint8_t NUM_PIEZOS = 1;
const uint8_t PINES_PIEZO[NUM_PIEZOS] = {A17};
const uint8_t NOTAS_MIDI[NUM_PIEZOS] = {36}; // C2
#else
const uint8_t NUM_PIEZOS = 12;
const uint8_t PINES_PIEZO[NUM_PIEZOS] = {
  A0, A1, A2, A3, A6, A7, A8, A9, A10, A11, A12, A13
};
// A0 y A2 conservan C2 y D2. A9 se cambia temporalmente de G2 a F#2.
const uint8_t NOTAS_MIDI[NUM_PIEZOS] = {
  36, 37, 38, 39, 40, 41, 42, 42, 44, 45, 46, 47
};
#endif

// ---------------- MIDI y deteccion de golpes ----------------
const uint8_t  CANAL_MIDI         = 1;
const uint32_t PERIODO_US         = 50;   // 20 kHz total; ~1.67 kHz por piezo con 12 entradas
const uint16_t UMBRAL_GOLPE       = 400;  // ADC a 12 bits: 0..4095
const uint16_t UMBRAL_REARME      = 100;  // debe bajar de aqui antes de aceptar otro golpe
const uint32_t VENTANA_PICO_US    = 2000; // captura el pico real del golpe
const uint32_t DURACION_NOTA_MS   = 25;   // noteOff automatico

// ---------------- Buffer circular (productor: ISR, consumidor: loop) ----------------
#define BUF_SIZE 1024                     // debe ser potencia de 2
#define BUF_MASK (BUF_SIZE - 1)

struct Muestra {
  uint16_t valor;
  uint8_t canal;
};

volatile Muestra buf[BUF_SIZE];
volatile uint32_t head = 0;               // solo lo escribe la ISR
uint32_t tail = 0;                        // solo lo escribe loop()
volatile uint8_t canalEnConversion = 0;
uint8_t proximoCanal = 0;

// Estado independiente de cada piezo.
bool capturandoGolpe[NUM_PIEZOS] = {};
bool esperandoRearme[NUM_PIEZOS] = {};
bool notaActiva[NUM_PIEZOS] = {};
uint16_t picoGolpe[NUM_PIEZOS] = {};
uint32_t inicioGolpeUs[NUM_PIEZOS] = {};
uint32_t inicioNotaMs[NUM_PIEZOS] = {};

// ISR: se ejecuta cuando ADC0 termina la conversion iniciada por disparar().
void adc0_isr() {
  uint32_t indice = head & BUF_MASK;
  buf[indice].valor = adc->adc0->readSingle(); // readSingle() tambien limpia la bandera
  buf[indice].canal = canalEnConversion;
  head++;
}

// Cada tick del temporizador selecciona el siguiente piezo y comienza su conversion.
void disparar() {
  canalEnConversion = proximoCanal;
  adc->adc0->startSingleRead(PINES_PIEZO[canalEnConversion]);
  proximoCanal++;
  if (proximoCanal >= NUM_PIEZOS) {
    proximoCanal = 0;
  }
}

void setup() {
  for (uint8_t i = 0; i < NUM_PIEZOS; i++) {
    pinMode(PINES_PIEZO[i], INPUT_DISABLE); // desactiva el keeper digital
  }

  adc->adc0->setAveraging(1);
  adc->adc0->setResolution(12);
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
  adc->adc0->enableInterrupts(adc0_isr);

  timerMuestreo.begin(disparar, PERIODO_US);
}

void procesarMuestra(uint8_t canal, uint16_t valor) {
  // Rearma solo cuando la vibracion residual ha bajado suficientemente.
  if (esperandoRearme[canal] && valor < UMBRAL_REARME) {
    esperandoRearme[canal] = false;
  }

  // Inicia la captura de un golpe nuevo.
  if (!capturandoGolpe[canal] && !esperandoRearme[canal] && valor >= UMBRAL_GOLPE) {
    capturandoGolpe[canal] = true;
    picoGolpe[canal] = valor;
    inicioGolpeUs[canal] = micros();
  }

  if (!capturandoGolpe[canal]) {
    return;
  }

  if (valor > picoGolpe[canal]) {
    picoGolpe[canal] = valor;
  }

  if (micros() - inicioGolpeUs[canal] < VENTANA_PICO_US) {
    return;
  }

  uint8_t velocidad = map(picoGolpe[canal], UMBRAL_GOLPE, 4095, 1, 100);
  velocidad = constrain(velocidad, 1, 127);
  uint8_t nota = NOTAS_MIDI[canal];

  // Cierra solo la nota de este piezo antes de volverla a disparar.
  if (notaActiva[canal]) {
    usbMIDI.sendNoteOff(nota, 0, CANAL_MIDI);
  }
  usbMIDI.sendNoteOn(nota, velocidad, CANAL_MIDI);

  notaActiva[canal] = true;
  inicioNotaMs[canal] = millis();
  capturandoGolpe[canal] = false;
  esperandoRearme[canal] = true;
}

void loop() {
  while (tail != head) {
    uint32_t indice = tail & BUF_MASK;
    uint16_t valor = buf[indice].valor;
    uint8_t canal = buf[indice].canal;
    tail++;
    procesarMuestra(canal, valor);
  }

  // Cada piezo mantiene su propio noteOff para permitir acordes y golpes simultaneos.
  for (uint8_t canal = 0; canal < NUM_PIEZOS; canal++) {
    if (notaActiva[canal] && millis() - inicioNotaMs[canal] >= DURACION_NOTA_MS) {
      usbMIDI.sendNoteOff(NOTAS_MIDI[canal], 0, CANAL_MIDI);
      notaActiva[canal] = false;
    }
  }
}
