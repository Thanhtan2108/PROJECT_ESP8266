#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/*
  config.h  (header-only)
  - Chứa cấu hình WiFi và Firebase Realtime Database (host + secret).
  - Đây là file chứa secret: KHÔNG nên push lên repo công khai.
  - Cách an toàn hơn: bỏ giá trị thật ra file .gitignored, hoặc dùng build_flags/system env.
*/

/* ----------------------------
   ===  EDIT THESE VALUES  ===
   ---------------------------- */

/* Hardware pins (change as needed) */
#define BUTTON_PIN D6
#define LED_PIN    D7

/* I2C device addresses */
#define OLED_ADDR       0x3C
#define MAX30102_ADDR   0x57

/* WiFi credentials */
static const char WIFI_SSID[]     = "P203&P204&P205";
static const char WIFI_PASSWORD[] = "102duylam";

/* Firebase Realtime Database host
   e.g. "https://your-project-id-default-rtdb.firebaseio.com" */
static const char FIREBASE_HOST[] = "https://heart-rate-spo2-52cb4-default-rtdb.asia-southeast1.firebasedatabase.app";

/* Firebase Database Secret (legacy) or database secret token.
   If you use a modern Firebase SDK with OAuth, you may leave this empty.
   WARNING: keep this secret private. */
static const char FIREBASE_AUTH[] = "z7nxX8FIyI2Ws92thgkoe23Mj3Vw2gXJnmxmrTMa";

/* How often to send a report to Firebase (ms) */
static constexpr unsigned long FIREBASE_REPORT_INTERVAL_MS = 3000UL;

/* Optional: root path in your RTDB where data will be pushed */
static const char FIREBASE_PATH_ROOT[] = "/health";

/* NTP time configuration */
static const char NTP_SERVER[] = "pool.ntp.org";
static const long GMT_OFFSET_SEC = 7 * 3600; // múi giờ VN (GMT+7)
static const int DAYLIGHT_OFFSET_SEC = 0;

/* ----------------------------
   ===  END EDITABLE PART  ===
   ---------------------------- */

#endif // CONFIG_H
