#include <FastLED.h>

// Configurações das fitas LED
#define NUM_LEDS_PER_STRIP 10   // Número de LEDs por fita (ajuste conforme necessário)
#define NUM_STRIPS 12           // Número de fitas (12 degraus)
#define BRIGHTNESS_LOW 3        // Brilho baixo (1%)
#define BRIGHTNESS_HIGH 128     // Brilho alto (50%)
#define LED_TYPE WS2812B        // Tipo de LED
#define COLOR_ORDER GRB         // Ordem de cor dos LEDs

// Pinos dos LEDs
const int LED_PIN_1 = 2;
const int LED_PIN_2 = 3;
const int LED_PIN_3 = 4;
const int LED_PIN_4 = 5;
const int LED_PIN_5 = 6;
const int LED_PIN_6 = 7;
const int LED_PIN_7 = 8;
const int LED_PIN_8 = 9;
const int LED_PIN_9 = 10;
const int LED_PIN_10 = 11;
const int LED_PIN_11 = 12;
const int LED_PIN_12 = 13;

// Configurações dos sensores PIR
#define NUM_SENSORS 15
#define SENSOR_PIN_START 22     // Pino inicial dos sensores PIR
#define DETECTION_DELAY 100     // Atraso em milissegundos entre as luzes ascendendo
#define KEEP_LIGHTS_ON 5        // Quantidade de degraus iluminados simultaneamente
#define FADE_DURATION 500       // Duração de fade em milissegundos

CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];
int sensorPins[NUM_SENSORS];
bool sensorStates[NUM_SENSORS];
unsigned long sensorLastTriggered[NUM_SENSORS];

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

// Variáveis de controle
int currentBrightness[NUM_STRIPS];
unsigned long lastUpdate = 0;
unsigned long lastSensorCheck = 0;

void setup() {
  Serial.begin(115200);  // Comunicação com o PC

  // Inicializa LEDs
  FastLED.addLeds<LED_TYPE, LED_PIN_1, COLOR_ORDER>(leds[0], NUM_LEDS_PER_STRIP); // Configura a fita 1
  FastLED.addLeds<LED_TYPE, LED_PIN_2, COLOR_ORDER>(leds[1], NUM_LEDS_PER_STRIP); // Configura a fita 2
  FastLED.addLeds<LED_TYPE, LED_PIN_3, COLOR_ORDER>(leds[2], NUM_LEDS_PER_STRIP); // Configura a fita 3
  FastLED.addLeds<LED_TYPE, LED_PIN_4, COLOR_ORDER>(leds[3], NUM_LEDS_PER_STRIP); // Configura a fita 4
  FastLED.addLeds<LED_TYPE, LED_PIN_5, COLOR_ORDER>(leds[4], NUM_LEDS_PER_STRIP); // Configura a fita 5
  FastLED.addLeds<LED_TYPE, LED_PIN_6, COLOR_ORDER>(leds[5], NUM_LEDS_PER_STRIP); // Configura a fita 6
  FastLED.addLeds<LED_TYPE, LED_PIN_7, COLOR_ORDER>(leds[6], NUM_LEDS_PER_STRIP); // Configura a fita 7
  FastLED.addLeds<LED_TYPE, LED_PIN_8, COLOR_ORDER>(leds[7], NUM_LEDS_PER_STRIP); // Configura a fita 8
  FastLED.addLeds<LED_TYPE, LED_PIN_9, COLOR_ORDER>(leds[8], NUM_LEDS_PER_STRIP); // Configura a fita 9
  FastLED.addLeds<LED_TYPE, LED_PIN_10, COLOR_ORDER>(leds[9], NUM_LEDS_PER_STRIP); // Configura a fita 10
  FastLED.addLeds<LED_TYPE, LED_PIN_11, COLOR_ORDER>(leds[10], NUM_LEDS_PER_STRIP); // Configura a fita 11
  FastLED.addLeds<LED_TYPE, LED_PIN_12, COLOR_ORDER>(leds[11], NUM_LEDS_PER_STRIP); // Configura a fita 12

  FastLED.setBrightness(BRIGHTNESS_LOW);
  for (int i = 0; i < NUM_STRIPS; i++) {
    for (int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      leds[i][j] = CRGB::Black;  // Inicia todos apagados
    }
    currentBrightness[i] = BRIGHTNESS_LOW;
  }
  
  // Inicializa sensores PIR
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensorPins[i] = SENSOR_PIN_START + i;
    pinMode(sensorPins[i], INPUT);
    sensorStates[i] = false;
    sensorLastTriggered[i] = 0;
  }
  
  FastLED.show();
}

void loop() {
  checarSensores();
  // Código adicional para controle dos LEDs e sensores
}

void checarSensores() {
  unsigned long currentTime = millis();
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (digitalRead(sensorPins[i]) == HIGH) {
      sensorStates[i] = true;
      sensorLastTriggered[i] = currentTime;
    } else {
      if (currentTime - sensorLastTriggered[i] > DETECTION_DELAY) {
        sensorStates[i] = false;
      }
    }
  }
  
  for (int i = 0; i < NUM_STRIPS; i++) {
    if (isLEDActive(i)) {
      atualizarLEDs(i);
    }
  }
}

bool algumSensorAtivo() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (sensorStates[i]) {
      return true;
    }
  }
  return false;
}

bool isLEDActive(int stripIndex) {
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (sensorToLEDMapping[i] == stripIndex && sensorStates[i]) {
      return true;
    }
  }
  return false;
}

void atualizarLEDs(int ledStripIndex) {
  // Apaga todos os LEDs da fita
  for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    leds[ledStripIndex][i] = CRGB::Black;
  }

  // Acende os LEDs da fita com efeito de fade
  for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    leds[ledStripIndex][i] = CRGB::White; // Pode ser substituído por qualquer cor desejada
  }

  // Configura o brilho da fita
  FastLED.setBrightness(currentBrightness[ledStripIndex]);
  FastLED.show();
}

