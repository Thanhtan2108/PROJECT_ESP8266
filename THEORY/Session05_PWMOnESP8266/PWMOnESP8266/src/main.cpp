#include <Arduino.h>

int ledPin = D2; // GPIO4

void setDutyCycle(int pin, float percent);

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
}

void loop() {
  for (int i = 0; i <= 1023; i++) {
    setDutyCycle(ledPin, i);
    delay(5);
  }
  for (int i = 1023; i >= 0; i--) {
    setDutyCycle(ledPin, i);
    delay(5);
  }
}

// dùng analogWrite() là API chuẩn để điều chế PWM
void setDutyCycle(int pin, float percent) {
  int value = percent / 100.0 * 1023;
  analogWrite(pin, value);
}
