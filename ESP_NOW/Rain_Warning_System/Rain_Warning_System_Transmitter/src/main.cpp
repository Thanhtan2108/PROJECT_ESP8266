/*
ESP8266
Rain Sensor 
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

#define analogPin A0

// Address MAC of ESP8266 Receiver - Can change to your receiver's MAC address
// Address is code HEX
uint8_t broadcastAddress[] = {};

// Structure example to send data
// Must match the receiver structure
typedef struct analog_messagedata {
  uint16_t RainDropSensor = 0;
  uint32_t time = 0;
} analog_messagedata;

// Create a struct_message called myData
analog_messagedata myanalogData;

unsigned long lastTime = 0;
unsigned long timerDelay = 2000;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery Success");
  } else {
    Serial.println("Delivery Fail");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(112500);
  pinMode(analogPin, INPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for send CB to
  // get the status of Transmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(20);

  myanalogData.RainDropSensor = analogRead(analogPin);
  Serial.print("Rain Drop Sensor Value: ");
  Serial.println(myanalogData.RainDropSensor);
  myanalogData.time = millis();
  Serial.print("Time: ");
  Serial.println(myanalogData.time);
  Serial.print("analogRead(analogPin): ");
  Serial.println(analogRead(analogPin));

  // Send message via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t *) &myanalogData, sizeof(myanalogData));
}
