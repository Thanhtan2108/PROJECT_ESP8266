#include <Arduino.h>

#define POT_PIN A0

// put function declarations here:

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);

  pinMode(POT_PIN, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int temp = analogRead(POT_PIN);
  Serial.print("Temp: ");
  Serial.println(temp);
  delay(1000);
}

// put function definitions here:
