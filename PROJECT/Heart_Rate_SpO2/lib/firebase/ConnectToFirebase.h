#ifndef CONNECT_TO_FIREBASE_H
#define CONNECT_TO_FIREBASE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "config.h"

class DisplayOLED; // forward declaration to avoid header coupling

class ConnectToFirebase {
public:
  ConnectToFirebase();
  bool begin();
  bool isConnected();
  bool sendData(float bpm, float spo2);
  bool ReadData(int &bpmOut, float &spo2Out);

private:
  void connectWiFi();
  void connectFirebase();
  void syncNTP();
  
  FirebaseData fbdo;
  FirebaseAuth auth;
  FirebaseConfig fbConfig;
  unsigned long lastSend = 0;
  unsigned long lastReadMs = 0;
};

#endif // CONNECT_TO_FIREBASE_H
