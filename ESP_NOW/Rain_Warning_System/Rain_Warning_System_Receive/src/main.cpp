/*
ESP8266
LCD I2C Display
Buzzer
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <LCD-I2C.h>

// Initialize I2C LCD, addr 0x27, 16 col, 2 rows
LCD_I2C lcd(0x27, 16, 2);

const int buzzer = D3;
const int led = D4;

// Structure to receive analog raindrop sensor data
typedef struct analog_messagedata {
  uint16_t RainDropSensor;
  uint16_t time = 0;
} analog_messagedata;

analog_messagedata myRainDropData;

String lastMessage = "";

volatile bool newData = false;
volatile double receiveRainDropData = 0.0;

void displayMessage(const String &message) {
  if (message != lastMessage) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
    lastMessage = message;
  }
}

// Callback when data is received
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myRainDropData, incomingData, sizeof(myRainDropData));
  receiveRainDropData = myRainDropData.RainDropSensor;
  newData = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);

  Wire.begin();
  lcd.begin(&Wire);
  lcd.display();
  lcd.backlight();
  displayMessage("Waiting Data...");

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  if (newData) {
    newData = false;
    double rain = receiveRainDropData;

    Serial.print("Send Value: ");
    Serial.println(rain);

    if (rain > 3500) {
      displayMessage("No Rain");
      noTone(buzzer);
      digitalWrite(led, LOW);
    } else {
      displayMessage("Rain Detected");
      tone(buzzer, 2000, 500);
      digitalWrite(led, HIGH);
    }
  }
}
