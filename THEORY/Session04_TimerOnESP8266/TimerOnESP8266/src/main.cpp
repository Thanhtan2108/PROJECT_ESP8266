#include <Arduino.h>

// put function declarations here:
void IRAM_ATTR timerInterrupt();
void IRAM_ATTR timerInterruptBlinkLed();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);

  // đăng ký ISR
  timer1_attachInterrupt(timerInterrupt);
  // bật và thiết lập các thông số cho timer
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
  // cấu hình ngưỡng bắt đầu đếm
  timer1_write(2500000); // 0.5s tương ứng với bộ chia tần 16
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
// ISR này có thể thực thi 1 task nào đó, chú ý ko dùng delay trong ISR
void IRAM_ATTR timerInterrupt() {
  Serial.print("Current Time: ");
  Serial.println(millis());
  Serial.println(" ms");
}

void IRAM_ATTR timerInterruptBlinkLed() {
  digitalWrite(2, !digitalRead(2));
}