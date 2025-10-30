#include <Arduino.h>

/*
 * DIY Pulse Oximeter
 * Author: ItzzInfinity                                                                                                       // Add your name here
 * Date: 26/11/24                                                                                                              // Add the creation date here
 * 
 * Description: This program utilizes the MAX30102 pulse oximeter sensor, an ESP8266 microcontroller, 
 * and an SSD1306 OLED display to create a simple pulse oximeter. The device measures heart rate 
 * and blood oxygen saturation (SpO2) levels and displays them on the OLED screen.
 * 
 * Libraries Used:
 * - DFRobot_MAX30102: Library for interfacing with the MAX30102 sensor.
 * - Wire: Library for I2C communication.
 * - Adafruit_GFX and Adafruit_SSD1306: Libraries for controlling the SSD1306 OLED display.
 */

// Include required libraries
#include <DFRobot_MAX30102.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Sensor and Display Configuration
DFRobot_MAX30102 particleSensor;                                                                                             // Initialize the MAX30102 sensor
#define SCREEN_WIDTH 128                                                                                                     // OLED display width, in pixels
#define SCREEN_HEIGHT 64                                                                                                     // OLED display height, in pixels
#define OLED_RESET     -1                                                                                                    // Reset pin for the OLED (-1 if using Arduino reset)
#define SCREEN_ADDRESS 0x3C                                                                                                 // I2C address for the OLED display (0x3C for 128x64)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);                                                   // Create SSD1306 display object

// Global Variables
int32_t SPO2;                                                                                                               // SpO2 (Blood Oxygen Saturation)
int8_t SPO2Valid;                                                                                                           // Flag indicating validity of SpO2 calculation
int32_t heartRate;                                                                                                          // Heart Rate
int8_t heartRateValid;                                                                                                      // Flag indicating validity of Heart Rate calculation

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(100);

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {                                                               // Start the OLED display
    Serial.println(F("SSD1306 allocation failed"));                                                                         // Error message if display initialization fails
    for (;;) ;                                                                                                              // Halt execution
  }
  display.clearDisplay();                                                                                                   // Clear the display

  // Initialize the MAX30102 sensor
  while (!particleSensor.begin()) {
    Serial.println("MAX30102 was not found");                                                                               // Error message if sensor initialization fails
    delay(1000);                                                                                                            // Retry every second
  }

  // Configure the MAX30102 sensor
  particleSensor.sensorConfiguration(
    /*ledBrightness=*/60,                                                                                                   // LED brightness
    /*sampleAverage=*/SAMPLEAVG_8,                                                                                          // Average samples
    /*ledMode=*/MODE_MULTILED,                                                                                              // Multi-LED mode
    /*sampleRate=*/SAMPLERATE_400,                                                                                          // Sample rate of 400 Hz
    /*pulseWidth=*/PULSEWIDTH_411,                                                                                          // Pulse width of 411 us
    /*adcRange=*/ADCRANGE_16384                                                                                             // ADC range of 16384
  );
}

void loop() {
  // Notify user to wait
  //  Serial.println(F("Wait about four seconds"));

  // Retrieve heart rate and SpO2 data from the MAX30102 sensor
  particleSensor.heartrateAndOxygenSaturation(
    /**SPO2=*/&SPO2,
    /**SPO2Valid=*/&SPO2Valid, 
    /**heartRate=*/&heartRate, 
    /**heartRateValid=*/&heartRateValid);

  // Display data on the serial monitor
  Serial.print(F("heartRate="));
  Serial.print(heartRate, DEC);
  Serial.print(F(", heartRateValid="));
  Serial.print(heartRateValid, DEC);
  Serial.print(F("; SPO2="));
  Serial.print(SPO2, DEC);
  Serial.print(F(", SPO2Valid="));
  Serial.println(SPO2Valid, DEC);

  // Clear the OLED display
  display.clearDisplay();

  // Display project title
  display.setTextSize(1);                                                                                                   // Set text size to small
  display.setTextColor(SSD1306_WHITE);                                                                                      // Set text color
  display.setCursor(0, 0);                                                                                                  // Set cursor position
  display.print("     Mini Project");
  display.display();

  // Display SpO2 value
  display.setCursor(0, 16);                                                                                                // Set cursor for SpO2 display
  display.print("Spo2: ");
  display.display();

  if (!SPO2Valid) {
    display.setTextSize(2);                                                                                                // Display invalid SpO2 as '--'
    display.print("--");
    display.display();
  } else {
    display.setTextSize(2);                                                                                                // Display valid SpO2 value
    display.print(SPO2, DEC);
    display.display();
  }

  // Display heart rate
  display.setCursor(0, 35);                                                                                                // Set cursor for heart rate display
  display.setTextSize(1);
  display.print(F("heartRate: "));
  display.display();

  if (!heartRateValid) {
    display.setTextSize(2);                                                                                                // Display invalid heart rate as '--'
    display.print("--");
    display.display();
  } else {
    display.setTextSize(2);                                                                                                // Display valid heart rate value
    display.print(heartRate, DEC);
    display.display();
  }

                                                                                                                           // Prompt user to place finger if both measurements are invalid
  if (!heartRateValid && !SPO2Valid) {
    display.setCursor(0, 50);
    display.setTextSize(1);
    display.print("Place your Finger");
    display.display();
  }
}
