#include <Arduino.h>
#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(100);

  // Khởi động WiFi ở chế độ để có thể đọc MAC của STA
  WiFi.mode(WIFI_STA);
  delay(100);

  // Địa chỉ MAC của STA (station mode)
  String macSTA = WiFi.macAddress();
  Serial.print("MAC address (STA): ");
  Serial.println(macSTA);

  // Khởi động WiFi ở chế độ để có thể đọc MAC của AP
  WiFi.mode(WIFI_AP);
  delay(100);

  String macAP = WiFi.softAPmacAddress();
  Serial.print("MAC address (AP): ");
  Serial.println(macAP);
}

void loop() {
  // Không cần làm gì thêm
}
