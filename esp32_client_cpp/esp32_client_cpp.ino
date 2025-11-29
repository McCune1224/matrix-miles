#include <Arduino.h>
#include <Adafruit_Protomatter.h>
#include <ctime>
#include "MatrixDisplay.h"
#include "WiFiManager.h"
#include "StravaClient.h"
#include "config.hpp"

#define USE_SERIAL Serial

// Matrix configuration for MatrixPortal M4 (64x32 RGB matrix)
#define MATRIX_CHAIN 1  // Number of matrix panels chained

// Matrix pins for MatrixPortal M4
uint8_t rgbPins[] = {7, 8, 9, 10, 11, 12};
uint8_t addrPins[] = {17, 18, 19, 20};
uint8_t clockPin = 14;
uint8_t latchPin = 15;
uint8_t oePin = 16;

// Create matrix object
Adafruit_Protomatter matrix(
  MATRIX_WIDTH, MATRIX_CHAIN,
  1, rgbPins,
  sizeof(addrPins), addrPins,
  clockPin, latchPin, oePin,
  true  // double buffering
);

// WiFi manager instance
WiFiManager wifiManager;

// Strava client instance
StravaClient* stravaClient = nullptr;

// Activity data - calendar days with activities
CalendarDay activityDays[31];
int activityCount = 0;

// Timing for periodic API calls (5 minutes)
const unsigned long API_FETCH_INTERVAL_MS = 5 * 60 * 1000;
unsigned long lastApiCallTime = 0;

void setup() {
  USE_SERIAL.begin(115200);
  USE_SERIAL.println("Matrix Miles - Calendar Display Test");

  randomSeed(analogRead(0));  // For random animations

  // Initialize matrix
  ProtomatterStatus status = matrix.begin();
  USE_SERIAL.printf("Matrix status: %d\n", status);

  // Initialize WiFi
  USE_SERIAL.println("Initializing WiFi...");
  if (wifiManager.connect(WIFI_SSID, WIFI_PASSWORD)) {
    USE_SERIAL.println("WiFi initialized successfully!");
    
    // Test connectivity to google.com (public DNS test)
    USE_SERIAL.println("\n=== Connectivity Tests ===");
    if (wifiManager.testConnectivity("google.com", 443)) {
      USE_SERIAL.println("[Test] ✓ Can reach google.com (internet working)");
    } else {
      USE_SERIAL.println("[Test] ✗ Cannot reach google.com (internet may be down)");
    }
    
    // Test connectivity to your API server
    USE_SERIAL.print("[Test] Testing API server: ");
    USE_SERIAL.println(SERVER_BASE_URL);
    // Extract just the hostname for testing
    String testHost = String(SERVER_BASE_URL);
    testHost.replace("https://", "");
    testHost.replace("http://", "");
    int slashPos = testHost.indexOf('/');
    if (slashPos > 0) {
      testHost = testHost.substring(0, slashPos);
    }
    if (wifiManager.testConnectivity(testHost.c_str(), 443)) {
      USE_SERIAL.println("[Test] ✓ Can reach API server on 443 (HTTPS)");
    } else {
      USE_SERIAL.println("[Test] ✗ Cannot reach API server on 443 (HTTPS)");
      USE_SERIAL.println("[Test] Trying HTTP on port 80...");
      if (wifiManager.testHTTPConnectivity(testHost.c_str())) {
        USE_SERIAL.println("[Test] ✓ Can reach API server on 80 (HTTP works, HTTPS may have cert issues)");
      } else {
        USE_SERIAL.println("[Test] ✗ Cannot reach API server on port 80 either (network/firewall issue)");
      }
    }
    USE_SERIAL.println("=== End Connectivity Tests ===\n");
    
    // Note: MatrixPortal M4 doesn't have an RTC. Time will be incorrect until
    // we implement time sync (e.g., from HTTP headers or NTP via SAMD21 RTC peripheral)
    // For now, we'll fetch the current month's data
    
    // Initialize Strava client after WiFi is connected
    stravaClient = new StravaClient(ESP32_API_KEY, SERVER_BASE_URL, USER_ID);
    
    // Use HTTP to avoid HTTPS certificate issues
    // If you want HTTPS, make sure your server's SSL certificate is in WiFiNINA's trust store
    stravaClient->setUseHTTP(true);
    
    // Test the API connection with a diagnostic GET request
    stravaClient->testConnection();
    
    // Sync time from server before fetching calendar data
    Serial.println("\nAttempting to synchronize time with server...");
    if (stravaClient->syncTimeFromServer()) {
      Serial.println("Time synchronized successfully!");
    } else {
      Serial.println("Failed to sync time - using hardcoded date (Nov 2025)");
    }
    
    // Fetch calendar data immediately
    fetchAndDisplayCalendar();
  } else {
    USE_SERIAL.println("WiFi initialization failed - continuing without network");
  }

  // Loading animation
  loadingAnimation();

  // Display calendar (with default month or fetched data)
  displayCalendar();
}

void loop() {
  // Maintain WiFi connection
  wifiManager.maintain();
  
  // Periodically fetch calendar data if WiFi is connected
  if (stravaClient && wifiManager.isConnected()) {
    if (millis() - lastApiCallTime >= API_FETCH_INTERVAL_MS) {
      fetchAndDisplayCalendar();
      lastApiCallTime = millis();
    }
  }
  
  delay(1000);
}

void fetchAndDisplayCalendar() {
  USE_SERIAL.println("Fetching calendar data from API...");
  
  // TODO: MatrixPortal M4 doesn't have RTC, so time() returns 1970
  // For now, hardcode the current month. In production, set this from:
  // 1. HTTP response headers (Date field)
  // 2. NTP via SAMD21 RTC peripheral
  // 3. User input via button configuration
  
  int year = 2025;
  int month = 11;  // November
  
  USE_SERIAL.print("Fetching for ");
  USE_SERIAL.print(month);
  USE_SERIAL.print("/");
  USE_SERIAL.println(year);
  
  // Fetch calendar data
  int count = stravaClient->fetchCalendarData(year, month, activityDays, 31);
  
  if (count > 0) {
    activityCount = count;
    USE_SERIAL.print("Successfully fetched ");
    USE_SERIAL.print(activityCount);
    USE_SERIAL.println(" days with activities");
    
    // Update display
    displayCalendarWithMonth(month, year);
  } else {
    USE_SERIAL.println("Failed to fetch calendar data");
  }
}