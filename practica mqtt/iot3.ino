#include <WiFi.h>
#include <PubSubClient.h>
#include <map>

const char *WIFI_SSID = "ZTE_2.4G_Dq52Ed";
const char *WIFI_PASS = "bXgt274i";

const char *MQTT_BROKER_HOST = "broker.hivemq.com";
const int MQTT_BROKER_PORT = 1883;

const char *MQTT_CLIENT_ID = "clienterandom";         // Unique CLIENT_ID
const char *PUBLISH_TOPIC = "TOPIC_SUBSCRIBEGRUPO8";  // TOPIC where ESP32 publishes
const char *SUBSCRIBE_TOPIC = "TOPIC_PUBLISHGRUPO8";  // TOPIC where ESP32 receives
const char *LED_STATS_TOPIC = "LED_STATS";            // Topic for LED statistics

bool stopLightsGame = true;  // Variable to indicate whether the light game should stop
bool gameStarted = false;    // Variable to indicate whether the game has started

unsigned long ledStats[4] = { 0 };  // Array to store LED statistics

unsigned long lastUpdate = 0;  // Variable to control graph update

const char* LED_MESSAGES[4][2] = {
  {"RED_LEDON", "RED_LEDOFF"},
  {"ORANGE_LEDON", "ORANGE_LEDOFF"},
  {"GREEN_LEDON", "GREEN_LEDOFF"},
  {"BLUE_LEDON", "BLUE_LEDOFF"}
};

WiFiClient wiFiClient;
PubSubClient mqttClient(wiFiClient);

// Class for controlling an individual LED
class LEDController {
private:
  int pin;
public:
  LEDController(int pinNumber) : pin(pinNumber) {
    pinMode(pin, OUTPUT);
  }

  void turnOn() {
    digitalWrite(pin, HIGH);
  }

  void turnOff() {
    digitalWrite(pin, LOW);
  }

  bool isOn() const {
    return digitalRead(pin) == HIGH;
  }

  void toggle() {
    digitalWrite(pin, !digitalRead(pin));
  }
};

// Array of LEDController instances for each LED
LEDController ledControllers[4] = {
  LEDController(32),  // Red LED
  LEDController(25),  // Orange LED
  LEDController(27),  // Green LED
  LEDController(12)   // Blue LED
};

void lightsGame() {
  unsigned long previousMillis = 0;  // Variable to store previous time
  const long interval = 500;         // Time interval between each iteration (333 ms in this case)
  int counter = 0;                   // Counter to determine current state of the lights

  // Loop while the lights are not stopped or the counter does not reach a certain value
  while (!stopLightsGame && counter < 50) {
    unsigned long currentMillis = millis();  // Get the current time

    // If the interval time has passed
    if (currentMillis - previousMillis >= interval) {
      // Update the previous time
      previousMillis = currentMillis;

      // Toggle the LED state and update LED statistics
      int ledIndex = counter % 4;  // Get the LED index based on the counter
      LEDController& led = ledControllers[ledIndex]; // Get the LED controller
      led.toggle();  // Toggle the LED state
      if (led.isOn()) {
        // If the LED is on, update the on time
        ledStats[ledIndex] += interval * 6.5;
      }

      counter++;  // Increment the counter
    }

    // Handle incoming messages while running the lights game
    mqttClient.loop();
  }

  // Turn off all the lights when exiting the loop
  for (int i = 0; i < 4; i++) {
    ledControllers[i].turnOff();
  }

  // Publish the LED statistics after the light game finishes
  publishLEDStats();
}

void publishLEDStats() {
  // Build a string with the LED statistics data
  String payload = String(ledStats[0]) + "," + String(ledStats[1]) + "," + String(ledStats[2]) + "," + String(ledStats[3]);
  // Publish the data to the corresponding MQTT topic
  mqttClient.publish(LED_STATS_TOPIC, payload.c_str());
}

void callback(const char *topic, byte *payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) message += (char)payload[i];
  if (String(topic) == SUBSCRIBE_TOPIC) {
    Serial.println("Message from topic " + String(topic) + ":" + message);

    // Realizar acciones basadas en el mensaje recibido
    if (message == "PLAYLEDS") {
      stopLightsGame = false;
      gameStarted = true;
      lightsGame();
    } else if (message == "STOPLEDS") {
      stopLightsGame = true;
    } else if (message == "OFFLEDS") {
      for (int i = 0; i < 4; i++) {
        ledControllers[i].turnOff();
      }
      gameStarted = false;
    } else {
      // Verificar mensajes para controlar los LED individualmente
      for (int i = 0; i < 4; i++) {
        if (message == LED_MESSAGES[i][0]) {
          ledControllers[i].turnOn();
        } else if (message == LED_MESSAGES[i][1]) {
          ledControllers[i].turnOff();
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

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

void loop() {
  if (mqttClient.connected()) {
    unsigned long now = millis();
    mqttClient.loop();

    // Update statistics every 5 seconds if the game has started
    if (gameStarted && now - lastUpdate >= 5000) {
      lightsGame();
      lastUpdate = now;
    }
  } else {
    Serial.println("MQTT broker not connected!");
    delay(2000);
  }
}
