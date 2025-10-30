#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= OLED CONFIG =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ================= PULSE SENSOR CONFIG =================
// Pulse sensor nối vào chân Analog A0 của ESP8266 (NodeMCU có 1 chân A0 duy nhất)
#define PULSE_PIN A0

// Các biến tính toán nhịp tim
int Signal;                
int Threshold = 512;       // ngưỡng phát hiện đỉnh xung (tùy module, có thể chỉnh lại)
unsigned long lastBeat = 0;
int BPM;                   
bool Pulse = false;

void setup() {
  Serial.begin(115200);

  // Khởi động OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Pulse Sensor Init..."));
  display.display();

  delay(2000);
}

void loop() {
  // Đọc tín hiệu analog từ Pulse Sensor
  Signal = analogRead(PULSE_PIN);

  // Kiểm tra xem có vượt ngưỡng để coi là nhịp tim hay không
  if (Signal > Threshold && Pulse == false) {
    Pulse = true;
    unsigned long currentBeat = millis();
    unsigned long beatInterval = currentBeat - lastBeat;
    lastBeat = currentBeat;

    if (beatInterval > 300 && beatInterval < 2000) {  // lọc khoảng thời gian hợp lệ
      BPM = 60000 / beatInterval;
      Serial.print("BPM: ");
      Serial.println(BPM);
    }
  }

  if (Signal < Threshold) {
    Pulse = false;
  }

  // ======= Hiển thị lên OLED =======
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Pulse Sensor"));

  display.setCursor(0, 16);
  display.setTextSize(2);
  display.print("BPM: ");
  if (BPM > 0) {
    display.println(BPM);
  } else {
    display.println("--");
  }

  // Vẽ sóng dạng bar graph đơn giản
  int barHeight = map(Signal, 0, 1023, 0, 64);
  display.fillRect(100, 64 - barHeight, 20, barHeight, SSD1306_WHITE);

  display.display();

  delay(20); // delay nhỏ để hiển thị mượt
}
