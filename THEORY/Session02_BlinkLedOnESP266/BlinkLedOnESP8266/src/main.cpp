#include <Arduino.h>

#define LED_PIN 2
// put function declarations here:
void blinkLed();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);

  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  blinkLed();
}

// put function definitions here:
void blinkLed() {
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
