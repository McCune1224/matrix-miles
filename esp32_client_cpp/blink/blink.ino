#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

#define USE_SERIAL Serial

// Configuration - API Key
#define ESP32_API_KEY "9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6"

// Configuration - Server URLs
// Set to true for production, false for test
#define USE_PRODUCTION true

#if USE_PRODUCTION
const char* SERVER_URL = "https://your-production-server.com/api/endpoint";
#else
const char* SERVER_URL = "https://your-test-server.com/api/endpoint";
#endif

// Alternative: Define both and choose at runtime
const char* PRODUCTION_URL = "https://your-production-server.com/api/endpoint";
const char* TEST_URL = "https://your-test-server.com/api/endpoint";


WiFiMulti wifiMulti;

void setup() {

  USE_SERIAL.begin(115200);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  wifiMulti.addAP();
}

void loop() {
  // wait for WiFi connection
  if ((wifiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;

    USE_SERIAL.print("[HTTP] begin...\n");
    // Use the configured server URL
    http.begin(SERVER_URL);

    // Add API key header
    http.addHeader("X-API-Key", ESP32_API_KEY);

    USE_SERIAL.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {


      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        USE_SERIAL.println(payload);
      }
    } else {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

  delay(5000);
}
