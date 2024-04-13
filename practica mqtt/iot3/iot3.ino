#include <WiFi.h>
#include <PubSubClient.h>

const char * WIFI_SSID = "LOPEZ";
const char * WIFI_PASS = "76486651NL";

const char * MQTT_BROKER_HOST = "broker.hivemq.com";
const int MQTT_BROKER_PORT = 1883;

const char * MQTT_CLIENT_ID = "clienterandom";       // Unique CLIENT_ID
const char * PUBLISH_TOPIC = "TOPIC_SUBSCRIBEGRUPO8";  // TOPIC where ESP32 publishes

const char * SUBSCRIBE_TOPIC = "TOPIC_PUBLISHGRUPO8"; // TOPIC where ESP32 receive

int ledPins[4] = {32, 25, 27, 12}; //rojo, naranja, verde, azul.

void lightsGame()
{
  // Blink all LEDs in sequence
  for (int i = 0; i < 3; i++)
  {
    // Red LED on
    digitalWrite(ledPins[0], HIGH);
    delay(500);
    digitalWrite(ledPins[0], LOW);

    // Orange LED on
    digitalWrite(ledPins[1], HIGH);
    delay(500);
    digitalWrite(ledPins[1], LOW);

    // Green LED on
    digitalWrite(ledPins[2], HIGH);
    delay(500);
    digitalWrite(ledPins[2], LOW);

    // Blue LED on
    digitalWrite(ledPins[3], HIGH);
    delay(500);
    digitalWrite(ledPins[3], LOW);
  }
}

WiFiClient wiFiClient;
PubSubClient mqttClient(wiFiClient);

void callback(const char * topic, byte * payload, unsigned int length) 
{
  String message;
  for (int i = 0; i < length; i++) message += String((char) payload[i]);
  if (String(topic) == SUBSCRIBE_TOPIC) 
  {
    Serial.println("Message from topic " + String(topic) + ":" + message);
    
    if (message == "RED_LEDON") 
    {
      digitalWrite(ledPins[0], HIGH);
    } 
    else if (message == "RED_LEDOFF") 
    {
      digitalWrite(ledPins[0], LOW);
    } 
    else if (message == "BLUE_LEDON") 
    {
      digitalWrite(ledPins[3], HIGH);
    }
    else if (message == "BLUE_LEDOFF") 
    {
      digitalWrite(ledPins[3], LOW);
    }

    else if (message == "GREEN_LEDON") 
    {
      digitalWrite(ledPins[2], HIGH);
    }
    
    else if (message == "GREEN_LEDOFF") 
    {
      digitalWrite(ledPins[2], LOW);
    }

    else if (message == "ORANGE_LEDON") 
    {
      digitalWrite(ledPins[1], HIGH);
      
    } 
    else if (message == "ORANGE_LEDOFF") 
    {
      digitalWrite(ledPins[1], LOW);
    }
    
    else if (message == "PLAYLEDS") 
    {
      lightsGame();
    }
    else if (message == "OFFLEDS") 
    {
      
    }
  }
}

void setup() 
{
  Serial.begin(115200);
  
  for (int i = 0; i < 4; i++)
  {
      pinMode(ledPins[i], OUTPUT);
  }
    
  Serial.print("Connecting to " + String(WIFI_SSID));
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
  mqttClient.setCallback(callback);

  Serial.print("Connecting to " + String(MQTT_BROKER_HOST));
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println(" Connected!");
    mqttClient.subscribe(SUBSCRIBE_TOPIC);
  }  
}

unsigned char counter = 0;

unsigned long previousPublishMillis = 0;

void loop() 
{
  if (mqttClient.connected()) 
  {
    unsigned long now = millis();
    mqttClient.loop();
  }
  else 
  {
    Serial.println("MQTT broker not connected!");
    delay(2000);
  }
}
