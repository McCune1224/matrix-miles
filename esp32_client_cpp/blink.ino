#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // Install via Library Manager if not present
#include "config.h"       // Contains WiFi credentials and API keys

#define USE_SERIAL Serial

// Timing
const unsigned long FETCH_INTERVAL_MS = 10000;  // 5 minutes in milliseconds
unsigned long lastFetchTime = 0;

WiFiMulti wifiMulti;

void setup() {
  USE_SERIAL.begin(115200);
  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println("=================================");
  USE_SERIAL.println("Strava Activity Fetcher for ESP32");
  USE_SERIAL.println("=================================");

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  // Add WiFi network
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  USE_SERIAL.println("[SETUP] Connecting to WiFi...");
}

void fetchActivities() {
  HTTPClient http;

  USE_SERIAL.println("\n[HTTP] Fetching recent activities...");

  // Build the full URL
  String url = String(SERVER_BASE_URL) + "/activities/calendar/1/2025/11";
  // String url = String(SERVER_BASE_URL) + "/stats/" + String(USER_ID);

  // Configure HTTP client
  http.begin(url);
  http.addHeader("X-API-Key", ESP32_API_KEY);
  http.setTimeout(15000);  // 15 second timeout

  USE_SERIAL.printf("[HTTP] GET %s\n", url.c_str());

  // Send GET request
  int httpCode = http.GET();

  if (httpCode > 0) {
    USE_SERIAL.printf("[HTTP] Response code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();

      USE_SERIAL.println("\n========== ACTIVITIES JSON ==========");
      USE_SERIAL.println(payload);
      USE_SERIAL.println("=====================================\n");

      // Optional: Parse JSON for better formatting
      // Uncomment if ArduinoJson library is installed
      DynamicJsonDocument doc(8192);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        JsonArray activities = doc.as<JsonArray>();
        USE_SERIAL.printf("Found %d activities:\n\n", activities.size());

        int count = 0;
        for (JsonObject activity : activities) {
          count++;
          USE_SERIAL.printf("Activity #%d:\n", count);
          USE_SERIAL.printf("  Name: %s\n", activity["name"].as<const char *>());
          USE_SERIAL.printf("  Type: %s\n", activity["type"].as<const char *>());
          USE_SERIAL.printf("  Distance: %.2f km\n", activity["distance"].as<float>() / 1000.0);
          USE_SERIAL.printf("  Moving Time: %d seconds\n", activity["moving_time"].as<int>());
          USE_SERIAL.printf("  Start Date: %s\n", activity["start_date"].as<const char *>());
          USE_SERIAL.println();
        }
      } else {
        USE_SERIAL.printf("[JSON] Parse error: %s\n", error.c_str());
      }

    } else {
      USE_SERIAL.printf("[HTTP] Non-OK status code: %d\n", httpCode);
    }
  } else {
    USE_SERIAL.printf("[HTTP] GET failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void loop() {
  // Check WiFi connection
  if (wifiMulti.run() == WL_CONNECTED) {
    unsigned long currentTime = millis();

    // Fetch on first run or after interval
    if (lastFetchTime == 0 || (currentTime - lastFetchTime >= FETCH_INTERVAL_MS)) {
      fetchActivities();
      lastFetchTime = currentTime;

      unsigned long nextFetch = FETCH_INTERVAL_MS / 1000;
      USE_SERIAL.printf("[INFO] Next fetch in %lu seconds\n\n",
                        nextFetch);
    }

    // Small delay to prevent busy-waiting
    delay(1000);

  } else {
    USE_SERIAL.println("[WiFi] Not connected, waiting...");
    delay(5000);
  }
}
