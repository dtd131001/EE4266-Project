#include "DHT.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>
#include <AsyncUDP.h>

DHT dht(18,DHT11);

IPAddress ip(192,168,1,214);
const char* ssid     = "Ngoi Nha Hanh Phuc";
const char* password = "giadinhhanhphuc";

WiFiUDP udp;
Coap coap(udp);
AsyncUDP _udp;

// LED STATE
bool LEDSTATE;

// void listenUdp();

void listenUdp() {
  if (_udp.listen(5684)) {
      // Serial.println("oke");

      _udp.onPacket([](AsyncUDPPacket packet) {
      
        String str = (const char*)packet.data();
        Serial.println(str);
        
        if (str == "1") {
          Serial.println("oke");
          pinMode(2, OUTPUT);
          digitalWrite(2, HIGH);
        } else {
          digitalWrite(2, LOW);
        }
      });
  }
}

void setup() {
  // put your setup code here, to run once:
  dht.begin();
  Serial.begin(9600);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  coap.start();
}

void loop() {

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (t > 30){
    pinMode(23, OUTPUT);            //máy bơm
    digitalWrite(23, HIGH);
  }
  else{
    digitalWrite(23, LOW);
  }

  if(isnan(h)||isnan(t)){
    Serial.println("Doc tin hieu loi");
    delay(2000);
    return;
  }
  // Serial.print("Do am: ");
  // Serial.print(h);
  Serial.print("Nhiet do: ");
  Serial.print(t);
  Serial.println("*C");
  Serial.println("Send Request");
  uint16_t msgid1 = coap.put(IPAddress(192,168,1,214), 5683, "temp", String(t).c_str());
 
  listenUdp();
  delay(1000);
  coap.loop();
}
