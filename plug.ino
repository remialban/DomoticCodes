#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "WIFI_SSID"
#define wifi_password "WIFI_PASSWORD"

#define mqtt_server "MQTT_SERVER"
#define mqtt_user "guest"
#define mqtt_password "guest"

char message_buff[100];

WiFiClient espClient;
PubSubClient client(espClient);

bool is_on_output_1 = false;
bool is_on_output_2 = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  Serial.begin(9600);
  off_output_1();
  off_output_2();

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connection to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connection established");
  Serial.print("=> IP Adress: ");
  Serial.print(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connection to MQTT server...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("OK");
    } else {
      Serial.print("KO, error : ");
      Serial.print(client.state());
      Serial.println("We wait 5 seconds before starting over");
      delay(5000);
    }
  }
  
  client.subscribe("plug/action-output-1");
  client.subscribe("plug/action-output-2");
}

void on_output_1()
{
  digitalWrite(D1, LOW);
  is_on_output_1 = true;
}

void on_output_2()
{
  digitalWrite(D2, LOW);
  is_on_output_2 = true;
}

void off_output_1()
{
  digitalWrite(D1, HIGH);
  is_on_output_1 = false;
}

void off_output_2()
{
  digitalWrite(D2, HIGH);
  is_on_output_2 = false;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  
  Serial.println("coucou");
}

void test()
{
  if (is_on_output_1)
  {
      client.publish("plug/info-output-1", String("1").c_str(), false);
  } else
  {
      client.publish("plug/info-output-1", String("0").c_str(), false);
  }

  if (is_on_output_2)
  {
      client.publish("plug/info-output-2", String("1").c_str(), false);
  } else
  {
      client.publish("plug/info-output-2", String("0").c_str(), false);
  }
}
void callback(char* topic, byte* payload, unsigned int length) {

  int i = 0;
  Serial.println("Received message =>  topic: " + String(topic));
  Serial.print(" | lenght: " + String(length,DEC));
  
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  
  String msgString = String(message_buff);
  
  Serial.println("Payload: " + msgString);

  if (String(topic) == "plug/action-output-1")
  {
    if (msgString == "1")
    {
      on_output_1();
    }
    if (msgString == "0")
    {
      off_output_1();
    }
  }

  if (String(topic) == "plug/action-output-2")
  {
    if (msgString == "1")
    {
      on_output_2();
    }
    if (msgString == "0")
    {
      off_output_2();
    }
  }
  test();
}
