#include <Arduino.h>

#define BUTTON_PIN 14   // GPIO14 (D5)

volatile bool interruptFlag = false; // cờ báo ngắt

void IRAM_ATTR pressButtonToInterrupt() {
  // ISR: thật ngắn — chỉ đặt cờ
  interruptFlag = true;
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(BUTTON_PIN, INPUT_PULLUP); // nút nối GND, dùng pullup nội bộ

  int interruptNumber = digitalPinToInterrupt(BUTTON_PIN);
  attachInterrupt(interruptNumber, pressButtonToInterrupt, FALLING);
}

void loop() {
  static unsigned long lastHandled = 0;
  const unsigned long debounceMs = 50;

  if (interruptFlag) {
    // reset cờ ngay để không xử lý lặp nhiều lần
    interruptFlag = false;

    unsigned long now = millis();
    // debounce: chỉ in khi đủ thời gian từ lần in trước
    if (now - lastHandled >= debounceMs) {
      lastHandled = now;
      Serial.println("Interrupt activated");
    }
  }

  // done: có thể thêm công việc khác ở đây
}
