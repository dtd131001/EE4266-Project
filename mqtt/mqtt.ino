#include "DHT.h"
#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>

#define WIFI_SSID "Ngoi Nha Hanh Phuc"
#define WIFI_PASSWORD "giadinhhanhphuc"

// Mosquitto MQTT Broker
#define MQTT_HOST IPAddress(192, 168, 1, 214)
#define MQTT_PORT 1883

// Temperature MQTT Topics
#define MQTT_PUB_TEMP "indoor/temperature"
#define MQTT_PUB_HUM  "indoor/humidity"

//MQTT Subscribe Topics
#define MQTT_SUB_DIGITAL "esp32/digital/#"

// Digital pin connected to the DHT sensor
#define DHTPIN 4  

// Uncomment whatever DHT sensor type you're using
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)   

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Variables to hold sensor readings
float temp;
float hum;

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 1000;        // Interval at which to publish sensor readings

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub1 = mqttClient.subscribe(MQTT_SUB_DIGITAL, 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub1);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  dht.begin();
  
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.onMessage(onMqttMessage);

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  // If your broker requires authentication (username and password), set them below
  //mqttClient.setCredentials("REPlACE_WITH_YOUR_USER", "REPLACE_WITH_YOUR_PASSWORD");
  connectToWifi();
}

  void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void loop() {
  unsigned long currentMillis = millis();
  // Every X number of seconds (interval = 10 seconds) 
  // it publishes a new MQTT message
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    // New DHT sensor readings
    hum = dht.readHumidity();
    // Read temperature as Celsius (the default)
    temp = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //temp = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(temp) || isnan(hum)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    
    // Publish an MQTT message on topic esp32/dht/temperature
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_TEMP, packetIdPub1);
    Serial.printf("Message: %.2f \n", temp);

    // Publish an MQTT message on topic esp32/dht/humidity
    uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(hum).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_HUM, packetIdPub2);
    Serial.printf("Message: %.2f \n", hum);
  }
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  // Do whatever you want when you receive a message
  
  // Save the message in a variable
  String receivedMessage;
  for (int i = 0; i < len; i++) {
    Serial.println((char)payload[i]);
    receivedMessage += (char)payload[i];
  }
  // Save topic in a String variable
  String receivedTopic = String(topic);  
  Serial.print("Received Topic: ");
  Serial.println(receivedTopic);

  // Check which GPIO we want to control
  int stringLen = receivedTopic.length();
  // Get the index of the last slash
  int lastSlash = receivedTopic.lastIndexOf("/");
  // Get the GPIO number (it's after the last slash "/")
  // esp/digital/GPIO
  String gpio = receivedTopic.substring(lastSlash+1, stringLen);
  Serial.print("DIGITAL GPIO: ");
  Serial.println(gpio);
  Serial.print("STATE: ");
  Serial.println(receivedMessage);

  // Check if it is DIGITAL
  if (receivedTopic.indexOf("digital") > 0) {
    //Set the specified GPIO as output
    pinMode(gpio.toInt(), OUTPUT);
    //Control the GPIO
    if (receivedMessage == "1"){
      digitalWrite(gpio.toInt(), HIGH);
    }
    else{
      digitalWrite(gpio.toInt(), LOW);
    }
  }
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}

