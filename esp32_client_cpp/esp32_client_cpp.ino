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
     USE_SERIAL.println("WiFi connected!");
     
     // Initialize Strava client after WiFi is connected
     stravaClient = new StravaClient(ESP32_API_KEY, SERVER_BASE_URL, USER_ID);
     stravaClient->setUseHTTP(true);
     
     // Sync time from server first
     showLoadingStatus("Syncing time...");
     if (stravaClient->syncTimeFromServer()) {
       USE_SERIAL.println("Time synced!");
     }
     
     // Sync activities with server
     showLoadingStatus("Syncing activities...");
     if (stravaClient->syncActivitiesWithServer()) {
       USE_SERIAL.println("Activities synced!");
     }
     
     // Fetch and display calendar data
     showLoadingStatus("Loading calendar...");
     fetchAndDisplayCalendar();
   } else {
     USE_SERIAL.println("WiFi failed - displaying default calendar");
   }
   
   // Display calendar
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
    // Get date from server sync first, then fall back to system time
    int day = 1, month = 1, year = 1970;
    
    if (stravaClient != nullptr) {
      stravaClient->getSyncedDate(day, month, year);
    }
    
    // If still invalid, try system time
    if (year <= 1970) {
      time_t now = time(nullptr);
      struct tm* timeinfo = localtime(&now);
      year = timeinfo->tm_year + 1900;
      month = timeinfo->tm_mon + 1;
    }
    
    // Fetch calendar data
    int count = stravaClient->fetchCalendarData(year, month, activityDays, 31);
    
    if (count > 0) {
      activityCount = count;
      displayCalendarWithMonth(month, year);
      transitionToCalendar();
    }
 }