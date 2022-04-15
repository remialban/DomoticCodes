#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Déclaration des sorties et des entrées
#define PIN_VENTILATION D0
#define PIN_RESISTANCE D1
#define PIN_TEMPERATURE D2

//Température
OneWire oneWire(PIN_TEMPERATURE);
DallasTemperature sensors(&oneWire);

// MQTT
const char* WIFI_SSID = "<WIFI_SSID>";
const char* WIFI_PASSWORD = "<WIFI_PASSWORD>";
const char* MQTT_SERVER = "<MQTT_BROKER>";

WiFiClient espClient;
PubSubClient client(espClient);

//Topic action

const char* TOPIC_ACTION_WORKING = "radiateur-gladys/action-working";
const char* TOPIC_ACTION_TEMPERATURE_SET = "radiateur-gladys/action-temperature-set";
const char* TOPIC_ACTION_DIFFERENTIAL_COLD = "radiateur-gladys/action-differential-cold";
const char* TOPIC_ACTION_MODE = "radiateur-gladys/action-mode";
const char* TOPIC_ACTION_TEMPERATURE = "radiateur-gladys/action-temperature";
const char* TOPIC_ACTION_DIFFERENTIAL_HEAT = "radiateur-gladys/action-differential-heat";

// Topic info
const char* TOPIC_INFO_TEMPERATURE = "radiateur-gladys/info-temperature";
const char* TOPIC_INFO_MODE = "radiateur-gladys/info-mode";
const char* TOPIC_INFO_WORKING = "radiateur-gladys/info-working";
const char* TOPIC_INFO_TEMPERATURE_SET = "radiateur-gladys/info-temperature-set";
const char* TOPIC_INFO_DIFFERENTIAL_HEAT = "radiateur-gladys/info-differential-heat";
const char* TOPIC_INFO_DIFFERENTIAL_COLD = "radiateur-gladys/info-differential-cold";
const char* TOPIC_INFO_VENTILATION = "radiateur-gladys/info-ventilation";
const char* TOPIC_INFO_RESISTANCE = "radiateur-gladys/info-resistance";

const char* TOPIC_INIT = "radiateur-gladys/init";

//Valeurs
const char* MODE_HEAT = "HEAT";
const char* MODE_COLD = "COLD";
const char* WORKING_ON = "1";
const char* WORKING_OFF = "0";
const char* TEMPERATURE_SET_UP = "UP";
const char* TEMPERATURE_SET_DOWN = "DOWN";
const char* DIFFERENTIAL_UP = "UP";
const char* DIFFERENTIAL_DOWN = "DOWN";

//Variables
bool isWorking = false;

String mode = "HEAT";
bool isVentilating = LOW;
bool isResisted = LOW;
int temperatureSet = 20;
float differentialHeat = 1;
float differentialCold = 1;
float temperature = 17;

bool etatVentilation = false;
bool etatResistance = false;
long millisResistedOff = 0;
long millisTemperature = 0;
float getTemperature() {
  sensors.requestTemperatures();
  return (sensors.getTempCByIndex(0));
}
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.hostname("ESP-RADIATEUR-GLADYS");
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

void reconnect() {
  // Loop until we're reconnected
  //while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(TOPIC_INIT, "init");
      // ... and resubscribe
      client.subscribe(TOPIC_ACTION_WORKING);
      client.subscribe(TOPIC_ACTION_TEMPERATURE_SET);
      client.subscribe(TOPIC_ACTION_DIFFERENTIAL_HEAT);
      client.subscribe(TOPIC_ACTION_DIFFERENTIAL_COLD);

      client.subscribe(TOPIC_ACTION_MODE);
      client.subscribe(TOPIC_ACTION_TEMPERATURE);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
 // }
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
  if (topic == TOPIC_ACTION_TEMPERATURE_SET) {
    temperatureSet = payload.toInt();
    client.publish(TOPIC_INFO_TEMPERATURE_SET, String(temperatureSet).c_str());

  }

  if (topic == TOPIC_ACTION_DIFFERENTIAL_HEAT) {
    differentialHeat = payload.toFloat();
    client.publish(TOPIC_INFO_DIFFERENTIAL_HEAT, String(differentialHeat).c_str());

  }

  if (topic == TOPIC_ACTION_DIFFERENTIAL_COLD) {
    
    differentialCold = payload.toFloat();
    client.publish(TOPIC_INFO_DIFFERENTIAL_HEAT, String(differentialCold).c_str());

  }

  if (topic == TOPIC_ACTION_WORKING) {
    if (payload == WORKING_ON) {
      isWorking = true;
      client.publish(TOPIC_INFO_WORKING, String(WORKING_ON).c_str());
    }
    if (payload == WORKING_OFF) {
      isWorking = false;
      client.publish(TOPIC_INFO_WORKING, String(WORKING_OFF).c_str());

    }
  }

  if (topic == TOPIC_ACTION_MODE) {
    if (payload == MODE_HEAT) {
      mode = MODE_HEAT;
      client.publish(TOPIC_INFO_MODE, String(MODE_HEAT).c_str());

    }
    if (payload == MODE_COLD) {
      mode = MODE_COLD;
      client.publish(TOPIC_INFO_MODE, String(MODE_COLD).c_str());

    }
  }
  if (topic == TOPIC_ACTION_TEMPERATURE) {
    temperature = payload.toFloat();
    Serial.println();
    Serial.print("Test : ");
    Serial.println(payload.toFloat());
  }


}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  sensors.begin();

  pinMode(PIN_RESISTANCE, OUTPUT);
  pinMode(PIN_VENTILATION, OUTPUT);

}

void refresh_outputs() {
  if (isVentilating) {
    digitalWrite(PIN_VENTILATION, HIGH);
      client.publish(TOPIC_INFO_VENTILATION, String(1).c_str());

  } else {
    Serial.println("Je suis dans la prmeiere condition millis");
    if (millis() - millisResistedOff > 120000) {
      Serial.println("Je suis dans la seconde condition millis");
      digitalWrite(PIN_VENTILATION, LOW);
        client.publish(TOPIC_INFO_VENTILATION, String(0).c_str());

    }
  }

  digitalWrite(PIN_RESISTANCE, isResisted);
  client.publish(TOPIC_INFO_RESISTANCE, String(isResisted).c_str());

}

void switchOnVentilation() {
  isVentilating = true;
}

void switchOnResistance() {
  switchOnVentilation();
  isResisted = true;
}

void switchOffResistance() {
  if(isResisted) {
      millisResistedOff = millis();
      
  }
  isResisted = false;
}

void switchOffVentilation() {
  switchOffResistance();
  isVentilating = false;
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  refresh_outputs();

  if (isWorking) {
    Serial.println("dans isworking");
    if (mode == MODE_HEAT) {
      Serial.println("dans mode heat");

      if (temperature <= temperatureSet - differentialHeat) {
      Serial.println("dans premiere cond");

        switchOnVentilation();
        switchOnResistance();
      }
      if (temperature >= temperatureSet) {
              Serial.println("dans seconde cond");

        switchOffResistance();
        switchOffVentilation();
      }
    } else {
      if (mode == MODE_COLD) {
        switchOffResistance();
        if (temperature >= temperatureSet + differentialCold) {
          switchOnVentilation();
        }
        if (temperature <= temperatureSet) {
          switchOffVentilation();
        }
      }
    }
  } else {
    switchOffResistance();
    switchOffVentilation();
  }
  
    if (millis() - millisTemperature > 30000)
    {
      millisTemperature = millis();
    temperature = getTemperature();
    client.publish(TOPIC_INFO_TEMPERATURE, String(temperature).c_str());

    }
  Serial.println();
  Serial.print("Temperature : ");
  Serial.println(temperature);
  Serial.print("TemperatureSSet : ");
  Serial.println(temperatureSet);
  Serial.print("Etat : ");
  Serial.println(isWorking);
  Serial.print("Mode : ");
  Serial.println(mode);
  Serial.print("Ventiler : ");
  Serial.println(isVentilating);
  Serial.print("Resister : ");
  Serial.println(isResisted);
  Serial.print("Differential heat : ");
  Serial.println(differentialHeat);
  Serial.print("Differential Cold : ");
  Serial.println(differentialCold);
  Serial.println(millis());
  Serial.println(millisResistedOff);
  delay(100);
}
