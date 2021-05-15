#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "WIFI_SSID"
#define wifi_password "WIFI_PASSWORD"

#define mqtt_server "IP_MOSQUITTO"

#define ONE_WIRE_BUS D2

OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

WiFiClient espClient;
PubSubClient client(espClient);

DallasTemperature sensors(&oneWire);

uint8_t sensor1[8] = { 0x28, 0x58, 0x2E, 0x07, 0xD6, 0x01, 0x3C, 0x5C };
uint8_t sensor2[8] = { 0x28, 0x4C, 0x89, 0x07, 0xD6, 0x01, 0x3C, 0xB9 };
uint8_t sensor3[8] = { 0x28, 0x26, 0x4B, 0x07, 0xD6, 0x01, 0x3C, 0x14 };
uint8_t sensor4[8] = { 0x28, 0xD6, 0xFE, 0x07, 0xD6, 0x01, 0x3C, 0xFB };

void setup(void)
{
  Serial.begin(9600);
  sensors.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop(void)
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  sensors.requestTemperatures();
  
  Serial.print("Sensor 1: ");
  client.publish("esp-cellier/temperature-sonde-1", String(sensors.getTempC(sensor1)).c_str(), true);

  
  Serial.print("Sensor 2: ");
  client.publish("esp-cellier/temperature-sonde-2", String(sensors.getTempC(sensor2)).c_str(), true);

  Serial.print("Sensor 3: ");
  client.publish("esp-cellier/temperature-sonde-3", String(sensors.getTempC(sensor3)).c_str(), true);


  Serial.print("Sensor 4: ");
  client.publish("esp-cellier/temperature-sonde-4", String(sensors.getTempC(sensor4)).c_str(), true);

  Serial.println();
  delay(60000);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connexion a ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connexion WiFi etablie ");
  Serial.print("=> Addresse IP : ");
  Serial.print(WiFi.localIP());
}
void reconnect() {
  //Boucle jusqu'à obtenur une reconnexion
  while (!client.connected()) {
    Serial.print("Connexion au serveur MQTT...");
    if (client.connect("ESP8266Client", "", "")) {
      Serial.println("OK");
    } else {
      Serial.print("KO, erreur : ");
      Serial.print(client.state());
      Serial.println(" On attend 5 secondes avant de recommencer");
      delay(5000);
    }
  }
}
