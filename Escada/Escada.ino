#include <FastLED.h>

// Configurações das fitas LED
#define NUM_LEDS_PER_STRIP 10   // Número de LEDs por fita (ajuste conforme necessário)
#define NUM_STRIPS 12           // Número de fitas (12 degraus)
#define BRIGHTNESS_LOW 3        // Brilho baixo (1%)
#define BRIGHTNESS_HIGH 128     // Brilho alto (50%)
#define LED_TYPE WS2812B        // Tipo de LED
#define COLOR_ORDER GRB         // Ordem de cor dos LEDs
const int ledPins[NUM_STRIPS] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}; // Pinos dos LEDs

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
  for (int i = 0; i < NUM_STRIPS; i++) {
    FastLED.addLeds<LED_TYPE, ledPins[i], COLOR_ORDER>(leds[i], NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS_LOW);
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
  unsigned long now = millis();

  // Checa sensores e atualiza LEDs
  if (now - lastSensorCheck >= 50) {  // Checa sensores a cada 50 ms
    checarSensores();
    lastSensorCheck = now;
  }

  // Atualiza o brilho gradualmente
  if (now - lastUpdate >= 20) {  // Atualiza LEDs a cada 20 ms
    for (int i = 0; i < NUM_STRIPS; i++) {
      if (currentBrightness[i] < BRIGHTNESS_HIGH && isLEDActive(i)) {
        currentBrightness[i] += (BRIGHTNESS_HIGH - BRIGHTNESS_LOW) * 20 / FADE_DURATION;
        if (currentBrightness[i] > BRIGHTNESS_HIGH) currentBrightness[i] = BRIGHTNESS_HIGH;
        FastLED.setBrightness(currentBrightness[i]);
        FastLED.show();
      }
    }
    lastUpdate = now;
  }
}

void checarSensores() {
  bool sensorTriggered = false;

  // Lê o estado de todos os sensores
  for (int i = 0; i < NUM_SENSORS; i++) {
    bool currentState = digitalRead(sensorPins[i]) == HIGH;
    if (currentState != sensorStates[i]) {
      sensorStates[i] = currentState;
      if (currentState) {
        sensorLastTriggered[i] = millis();
        atualizarLEDs(sensorToLEDMapping[i]);
      }
    }
  }
  
  // Se nenhum sensor estiver acionado, manter brilho mínimo
  if (!algumSensorAtivo()) {
    for (int i = 0; i < NUM_STRIPS; i++) {
      currentBrightness[i] = BRIGHTNESS_LOW;
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
  // Apaga todos os LEDs para reiniciar a sequência
  for (int i = 0; i < NUM_STRIPS; i++) {
    for (int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      leds[i][j] = CRGB::Black;
    }
    currentBrightness[i] = BRIGHTNESS_LOW;  // Reseta o brilho para todos os LEDs
  }

  // Ilumina o degrau atual e 2 antes/2 depois
  int startDegrau = max(ledStripIndex - 2, 0);
  int endDegrau = min(ledStripIndex + 2, NUM_STRIPS - 1);

  for (int i = startDegrau; i <= endDegrau; i++) {
    for (int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      leds[i][j] = CRGB::White;
    }
    currentBrightness[i] = BRIGHTNESS_HIGH; // Brilho máximo durante o movimento
  }
  FastLED.show();
}
