#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayOLED {
public:
  DisplayOLED(uint8_t width = 128, uint8_t height = 64, TwoWire *wire = &Wire, int8_t rst_pin = -1);

  bool begin(uint8_t i2cAddr = 0x3C);
  void showStatus(const String &line1, const String &line2 = String());
  void showVitals(int bpm, float spo2);
  void powerOn();
  void powerOff();

private:
  Adafruit_SSD1306 _display;
};

#endif
