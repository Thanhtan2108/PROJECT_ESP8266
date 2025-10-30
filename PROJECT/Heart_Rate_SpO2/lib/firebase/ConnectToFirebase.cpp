#include "ConnectToFirebase.h"

ConnectToFirebase::ConnectToFirebase() {}

bool ConnectToFirebase::begin() {
  connectWiFi();
  syncNTP();        // Đồng bộ thời gian NTP ngay khi khởi động
  connectFirebase();
  return isConnected();
}

/* -----------------------------
   Kết nối với Wifi
   ----------------------------- */
void ConnectToFirebase::connectWiFi() {
  Serial.println("\n📶 Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint8_t retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 30) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected!");
    Serial.print("📡 IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi connection failed!");
  }
}

/* -----------------------------
   Đồng bộ thời gian thực (NTP)
   ----------------------------- */
void ConnectToFirebase::syncNTP() {
  Serial.println("⏳ Syncing NTP time...");
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("⚠️ Failed to obtain NTP time!");
  } else {
    char timeStr[25];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.printf("✅ NTP synced successfully: %s\n", timeStr);
  }
}

/* -----------------------------
   Kết nối với Firebase
   ----------------------------- */
void ConnectToFirebase::connectFirebase() {
  Serial.println("🔥 Connecting to Firebase...");

  fbConfig.database_url = FIREBASE_HOST;
  fbConfig.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&fbConfig, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("✅ Firebase initialized successfully!");
}

bool ConnectToFirebase::isConnected() {
  return (WiFi.status() == WL_CONNECTED && Firebase.ready());
}

bool ConnectToFirebase::sendData(float bpm, float spo2) {
  if (millis() - lastSend < FIREBASE_REPORT_INTERVAL_MS)
    return false;

  lastSend = millis();

  if (!isConnected()) {
    Serial.println("⚠️ Firebase not connected, skipping send...");
    return false;
  }

  // --- Ghi nhận timestamp thực tế ---
  char timeStr[25];
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
  } else {
    strcpy(timeStr, "unknown_time");
  }

  // --- Lưu giá trị gốc để hiển thị debug ---
  float bpmRaw = bpm;
  float spo2Raw = spo2;

  // --- Chuyển đổi sang định dạng mong muốn ---
  int bpmInt = max(0, (int)round(bpm));                  // nhịp tim: số nguyên, không âm
  float spo2Rounded = ((int)(spo2 * 10 + 0.5)) / 10.0f;  // SpO2: 1 chữ số thập phân

  // --- Gửi trực tiếp vào node chính (realtime update) ---
  String pathBPM  = String(FIREBASE_PATH_ROOT) + "/BPM";
  String pathSpO2 = String(FIREBASE_PATH_ROOT) + "/SpO2";

  bool success = true;

  // --- Gửi BPM ---
  if (Firebase.RTDB.setInt(&fbdo, pathBPM.c_str(), bpmInt)) {
    Serial.printf("✅ [Firebase] BPM: %d BPM → %s (raw: %.2f) -> %d\n",
                  bpmInt, pathBPM.c_str(),bpmRaw, bpmInt);
  } else {
    Serial.printf("❌ [Firebase] BPM upload failed: %s\n", fbdo.errorReason().c_str());
    success = false;
  }

  // --- Gửi SpO2 ---
  if (Firebase.RTDB.setFloat(&fbdo, pathSpO2.c_str(), spo2Rounded)) {
    Serial.printf("✅ [Firebase] SpO₂: %.1f %% → %s (raw: %.2f) -> %.1f\n",
                  spo2Rounded, pathSpO2.c_str(), spo2Raw, spo2Rounded);
  } else {
    Serial.printf("❌ [Firebase] SpO₂ upload failed: %s\n", fbdo.errorReason().c_str());
    success = false;
  }

  // --- Hiển thị tóm tắt kết quả ---
  Serial.printf("[%s] ❤️ Heart Rate: %d BPM | 🩸 SpO₂: %.1f %%\n\n",
                timeStr, bpmInt, spo2Rounded);
                
  return success;
}

bool ConnectToFirebase::ReadData(int &bpmOut, float &spo2Out) {
  if (millis() - lastReadMs < FIREBASE_REPORT_INTERVAL_MS)
    return false;

  lastReadMs = millis();

  if (!isConnected()) {
    Serial.println("⚠️ Firebase not connected, skipping read...");
    return false;
  }

  String pathBPM  = String(FIREBASE_PATH_ROOT) + "/BPM";
  String pathSpO2 = String(FIREBASE_PATH_ROOT) + "/SpO2";

  bool ok = true;

  if (Firebase.RTDB.getInt(&fbdo, pathBPM.c_str())) {
    bpmOut = fbdo.intData();
  } else {
    ok = false;
  }

  if (Firebase.RTDB.getFloat(&fbdo, pathSpO2.c_str())) {
    spo2Out = fbdo.floatData();
  } else {
    ok = false;
  }

  return ok;
}
