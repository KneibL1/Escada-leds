#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configurações da rede Wi-Fi
const char* ssid = "SEU_SSID";
const char* password = "SUA_SENHA";

// Configurações do MQTT (ou outro protocolo para comunicar com a Alexa)
const char* mqtt_server = "BROKER_MQTT";
const int mqtt_port = 1883;
const char* mqtt_user = "USUARIO_MQTT";
const char* mqtt_password = "SENHA_MQTT";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);

  // Configura a comunicação serial com o Arduino Mega
  Serial1.begin(115200);  // RX/TX no ESP8266
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Transfere comandos recebidos da Alexa para o Arduino Mega
  while (client.available()) {
    String comando = client.readStringUntil('\n');
    Serial1.println(comando);  // Envia comando para o Arduino Mega
  }
}

void setup_wifi() {
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado ao Wi-Fi!");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      client.subscribe("comando/escada");
    } else {
      delay(5000);
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial1.println(message);  // Envia mensagem para o Arduino Mega
}
