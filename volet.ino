#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define OUTPUT_UP D1
#define OUTPUT_DOWN D2
#define INPUT_UP D6
#define INPUT_DOWN D7

#define WIFI_SSID "<YOUR_WIFI_SSID>"
#define WIFI_PASSWORD "<YOUR_WIFI_PASSWORD>"

#define MQTT_SERVER "<YOUR_MQTT_BROKER>"

WiFiClient espClient;
PubSubClient client(espClient);

String state;
String lastStateButton;

void setup() {
  Serial.begin(9600);
  
  pinMode(OUTPUT_UP, OUTPUT);
  pinMode(OUTPUT_DOWN, OUTPUT);
  pinMode(INPUT_UP, INPUT_PULLUP);
  pinMode(INPUT_DOWN, INPUT_PULLUP);

  setup_wifi();

  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);

  lastStateButton = button_state();
  stop_shutter();
}

void loop() {
  if (button_state() != lastStateButton)
  {
    lastStateButton = button_state();
    choose_action(lastStateButton);
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  Serial.println(state);
}

void choose_action(String &command)
{
  if (command == "OPEN")
  {
    open_shutter();
  } else if (command == "CLOSE")
  {
    close_shutter();
  } else if (command == "STOP") {
    stop_shutter();
  }
    client.publish("volet-remi/info-action", state.c_str());

}

void open_shutter()
{
  digitalWrite(OUTPUT_DOWN, LOW);
  digitalWrite(OUTPUT_UP, HIGH);
  state = "OPEN";
}

void close_shutter()
{
  digitalWrite(OUTPUT_UP, LOW);
  digitalWrite(OUTPUT_DOWN, HIGH);
  state = "CLOSE";
}

void stop_shutter()
{
  digitalWrite(OUTPUT_UP, LOW);
  digitalWrite(OUTPUT_DOWN, LOW);
  state = "STOP";
}

String button_state()
{
  if (!digitalRead(INPUT_UP) == HIGH)
  {
    return "OPEN";
  } else if (!digitalRead(INPUT_DOWN) == HIGH)
  {
    return "CLOSE";
  } else
  {
    return "STOP";
  }
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.hostname("ESP-VOLET-REMI");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(String topic, byte* message, unsigned int length) {

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String payload;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    payload += (char)message[i];
  }

  Serial.println();

  if (topic == "volet-remi/action")
  {
    choose_action(payload);
  }
}

void reconnect() {
  // Loop until we're reconnected
  //while (!client.connected()) {
  Serial.print("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "espvoletremi-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  if (client.connect(clientId.c_str())) {
    Serial.println("connected");
    client.subscribe("volet-remi/action");
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    delay(5000);
  }
  // }
}
