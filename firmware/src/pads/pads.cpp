#include <Arduino.h>
#include <ADC.h>
#include <math.h>
#include "pads.h"
#include "pines.h"
#include "config.h"

// ---------------- Buffer circular (productor: ISR, consumidor: loop) ----------------
#define BUF_SIZE 1024                 // potencia de 2
#define BUF_MASK (BUF_SIZE - 1)

struct Muestra {
  uint16_t valor;
  uint8_t  canal;
  uint32_t t;                         // micros() en el instante de la conversion
};

static ADC *adc = new ADC();
static IntervalTimer timerMuestreo;

static volatile Muestra buf[BUF_SIZE];
static volatile uint32_t head = 0;    // solo la ISR
static uint32_t tail = 0;             // solo loop()
static volatile uint8_t  canalEnConversion = 0;
static volatile uint32_t erroresADC = 0;
static uint8_t proximoCanal = 0;
static uint32_t overflows = 0;

// ---------------- Estado por pad ----------------
static bool     capturando[NUM_PADS]   = {};
static bool     esperandoRearme[NUM_PADS] = {};
static uint16_t pico[NUM_PADS]         = {};
static uint32_t inicioGolpeUs[NUM_PADS] = {};
static uint32_t ultimoGolpeUs[NUM_PADS] = {};
static bool     tuvoGolpe[NUM_PADS]    = {};

// Ultimo golpe global (para crosstalk)
static uint32_t crossT = 0;
static uint16_t crossPico = 0;
static bool     crossValido = false;

static uint16_t umbralGolpe = UMBRAL_GOLPE;
static uint8_t  lutVelocity[4096];
static PadHitCallback callback = nullptr;

// ---------------- ISR ----------------
static void adcIsr() {
  uint32_t i = head & BUF_MASK;
  buf[i].valor = adc->adc0->readSingle();   // tambien limpia la bandera
  buf[i].canal = canalEnConversion;
  buf[i].t = micros();
  head = head + 1;
}

static void disparar() {
  canalEnConversion = proximoCanal;
  if (!adc->adc0->startSingleRead(PINES_PAD[proximoCanal])) {
    erroresADC = erroresADC + 1;
  }
  proximoCanal++;
  if (proximoCanal >= NUM_PADS) proximoCanal = 0;
}

// ---------------- Velocity ----------------
static void construirLut() {
  for (int v = 0; v < 4096; v++) {
    if (v <= UMBRAL_GOLPE) { lutVelocity[v] = 1; continue; }
    float n = (float)(v - UMBRAL_GOLPE) / (float)(PICO_VELOCITY_MAX - UMBRAL_GOLPE);
    if (n > 1.0f) n = 1.0f;
    int vel = 1 + (int)lroundf(126.0f * powf(n, VELOCITY_GAMMA));
    lutVelocity[v] = (uint8_t)constrain(vel, 1, 127);
  }
}

// ---------------- API ----------------
void padsBegin(PadHitCallback alGolpe) {
  callback = alGolpe;
  construirLut();

  for (uint8_t i = 0; i < NUM_PADS; i++) {
    pinMode(PINES_PAD[i], INPUT_DISABLE);   // sin keeper digital
  }

  adc->adc0->setAveraging(1);
  adc->adc0->setResolution(12);
#if PADS_MUESTREO_LENTO
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::MED_SPEED);
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED);
#else
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
#endif
  adc->adc0->enableInterrupts(adcIsr);
  NVIC_SET_PRIORITY(IRQ_ADC1, PRIORIDAD_ADC_TIMER);   // adc0 de la lib = ADC1 del i.MX

  timerMuestreo.begin(disparar, PERIODO_MUESTREO_US);
  timerMuestreo.priority(PRIORIDAD_ADC_TIMER);
}

void padsSetSensibilidad(float escala) {
  escala = constrain(escala, 0.25f, 4.0f);
  umbralGolpe = (uint16_t)constrain((int)(UMBRAL_GOLPE * escala), 50, 3000);
}

uint32_t padsOverflows() { return overflows; }
uint32_t padsErroresADC() { return erroresADC; }

// ---------------- Procesado ----------------
static void procesar(uint8_t c, uint16_t valor, uint32_t t) {
  if (esperandoRearme[c] && valor < UMBRAL_REARME) esperandoRearme[c] = false;

  if (!capturando[c] && !esperandoRearme[c] && valor >= umbralGolpe) {
    // Tiempo muerto minimo entre golpes del mismo pad
    if (!tuvoGolpe[c] || (t - ultimoGolpeUs[c]) >= BLOQUEO_PAD_US) {
      capturando[c] = true;
      pico[c] = valor;
      inicioGolpeUs[c] = t;
    }
  }
  if (!capturando[c]) return;

  if (valor > pico[c]) pico[c] = valor;
  if ((t - inicioGolpeUs[c]) < VENTANA_PICO_US) return;

  // Fin de ventana: decidir si es golpe valido
  capturando[c] = false;
  esperandoRearme[c] = true;

  if (crossValido && (t - crossT) < CROSSTALK_US &&
      pico[c] < (uint16_t)(crossPico * CROSSTALK_RATIO)) {
    return;   // vibracion inducida por un pad vecino
  }

  crossT = t; crossPico = pico[c]; crossValido = true;
  ultimoGolpeUs[c] = t; tuvoGolpe[c] = true;

  if (callback) callback(c, lutVelocity[pico[c]]);
}

void padsUpdate() {
  uint32_t h = head;
  if ((h - tail) > BUF_SIZE) {            // la ISR nos alcanzo: descartar lo viejo
    overflows++;
    tail = h - BUF_SIZE;
  }
  while (tail != h) {
    uint32_t i = tail & BUF_MASK;
    uint16_t valor = buf[i].valor;
    uint8_t  canal = buf[i].canal;
    uint32_t t = buf[i].t;
    tail++;
    procesar(canal, valor, t);
  }
}
