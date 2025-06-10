#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "ssid";
const char* password = "senha";
const char* mqtt_server = "ip";
const int mqtt_port = 1883;

const char* sub_topics[] = {
  "casa/sala/led/set",
  "casa/cozinha/led/set",
  "casa/quarto1/led/set",
  "casa/quarto2/led/set",
  "casa/varanda/led/set"
};

const char* pub_topics[] = {
  "casa/sala/led/status",
  "casa/cozinha/led/status",
  "casa/quarto1/led/status",
  "casa/quarto2/led/status",
  "casa/varanda/led/status"
};

const uint8_t ledPins[] = { D0, D1, D2, D3, D4 }; 

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.print("Conectando na rede Wi-Fi: ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Wi-Fi conectado! IP: ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
    }
    msg.toUpperCase();

    Serial.print("Mensagem recebida [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(msg);

    for (int i = 0; i < 5; i++) {
        if (String(topic) == String(sub_topics[i])) {
        if (msg == "ON") {
            digitalWrite(ledPins[i], HIGH);
            Serial.println("→ LED ligado");
        } else if (msg == "OFF") {
            digitalWrite(ledPins[i], LOW);
            Serial.println("→ LED desligado");
        } else {
            Serial.println("→ Comando desconhecido");
        }
        client.publish(pub_topics[i], msg.c_str(), true);
        }
    }
}
void reconnect() {
    while (!client.connected()) {

        String clientId = "NodeMCU-";
        clientId += String(random(0xffff), HEX);
        Serial.print("Tentando conectar ao broker MQTT... ID = ");
        Serial.println(clientId);

        if (client.connect(clientId.c_str())) {
            Serial.println("Conectado ao broker!");

            for (int i = 0; i < 5; i++) {
                client.subscribe(sub_topics[i]);
                Serial.print("Inscrito no tópico: ");
                Serial.println(sub_topics[i]);
            }
        } else {
            Serial.print("Falha na conexão. Código de erro: ");
            Serial.print(client.state());
            Serial.println(" – tentando novamente em 5 segundos");
            delay(5000);
        }
    }
}
void setup() {
    Serial.begin(9600);
    delay(50);
    Serial.println();
    Serial.println("=== Iniciando NodeMCU + MQTT ===");

    for (int i = 0; i < 5; i++) {
        pinMode(ledPins[i], OUTPUT);
        digitalWrite(ledPins[i], LOW);
    }

    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}
void loop() {
    if (!client.connected()) {
        reconnect();
    }

    client.loop();
}