#include <FastLED.h>

// Configurações das fitas LED
#define NUM_LEDS_PER_STRIP 10 // Número de LEDs por fita (ajuste conforme necessário)
#define NUM_STRIPS 12         // Número de fitas (12 degraus)
#define BRIGHTNESS_LOW 3      // Brilho baixo (1%)
#define BRIGHTNESS_HIGH 128   // Brilho alto (50%)
#define LED_TYPE WS2812B      // Tipo de LED
#define COLOR_ORDER GRB       // Ordem de cor dos LEDs
const int ledPins[NUM_STRIPS] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}; // Pinos dos LEDs

// Configurações dos sensores PIR
#define NUM_SENSORS 15
#define SENSOR_PIN_START 22   // Pin inicial dos sensores PIR
#define DETECTION_DELAY 100   // Atraso em milissegundos entre as luzes ascendendo
#define KEEP_LIGHTS_ON 5      // Quantidade de degraus iluminados simultaneamente

CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];
int sensorPins[NUM_SENSORS];

// Vinculações entre sensores e fitas de LED
int sensorToLEDMapping[NUM_SENSORS] = {
  0, 0,  // Sensores 1 e 2 -> Fita 1 (Degrau 1)
  1,     // Sensor 3 -> Fita 2 (Degrau 2)
  2, 2, 2, // Sensores 4, 5 e 6 -> Fita 3 (Degrau 3)
  3,     // Sensor 7 -> Fita 4 (Degrau 4)
  4,     // Sensor 8 -> Fita 5 (Degrau 5)
  5,     // Sensor 9 -> Fita 6 (Degrau 6)
  6,     // Sensor 10 -> Fita 7 (Degrau 7)
  7,     // Sensor 11 -> Fita 8 (Degrau 8)
  8,     // Sensor 12 -> Fita 9 (Degrau 9)
  9,     // Sensor 13 -> Fita 10 (Degrau 10)
  10,    // Sensor 14 -> Fita 11 (Degrau 11)
  11     // Sensor 15 -> Fita 12 (Degrau 12)
};

void setup() {
  // Inicializa LEDs
  for (int i = 0; i < NUM_STRIPS; i++) {
    FastLED.addLeds<LED_TYPE, ledPins[i], COLOR_ORDER>(leds[i], NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS_LOW);
    for (int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      leds[i][j] = CRGB::Black;  // Inicia todos apagados
    }
  }
  
  // Inicializa sensores PIR
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensorPins[i] = SENSOR_PIN_START + i;
    pinMode(sensorPins[i], INPUT);
  }
  
  FastLED.show();
}

void loop() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (digitalRead(sensorPins[i]) == HIGH) {
      // Movimento detectado no sensor i
      int ledStripIndex = sensorToLEDMapping[i];
      iluminarDegraus(ledStripIndex);
    }
  }
}

void iluminarDegraus(int ledStripIndex) {
  // Apaga todos os LEDs para reiniciar a sequência
  for (int i = 0; i < NUM_STRIPS; i++) {
    for (int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      leds[i][j] = CRGB::Black;
    }
  }

  // Ilumina o degrau atual e 2 antes/2 depois
  int startDegrau = max(ledStripIndex - 2, 0);
  int endDegrau = min(ledStripIndex + 2, NUM_STRIPS - 1);

  for (int i = startDegrau; i <= endDegrau; i++) {
    for (int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      leds[i][j] = CRGB::White;
      FastLED.setBrightness(i == ledStripIndex ? BRIGHTNESS_HIGH : BRIGHTNESS_LOW);
    }
    FastLED.show();
    delay(DETECTION_DELAY);  // Atraso para criar o efeito de sequência
  }
}
