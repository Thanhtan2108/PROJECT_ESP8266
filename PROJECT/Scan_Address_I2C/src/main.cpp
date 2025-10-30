#include <Arduino.h>

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1); // SDA = D2 (GPIO4), SCL = D1 (GPIO5) trên NodeMCU ESP8266
  Serial.println("\nI2C Scanner bắt đầu...");
}

void loop() {
  byte error, address;
  int nDevices;

  Serial.println("Đang quét...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Thiết bị I2C tìm thấy tại địa chỉ 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");

      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Lỗi không xác định tại địa chỉ 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("Không tìm thấy thiết bị I2C nào\n");
  else
    Serial.println("Hoàn tất quét\n");

  delay(5000); // quét lại sau 5 giây
}
