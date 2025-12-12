#include <Arduino.h>
#include <Adafruit_Protomatter.h>
#include <ctime>

#include "config.hpp"
#include "ProtoDisplay.h"
#include "ButtonManager.h"
#include "PanelManager.h"
#include "SplashPanel.h"
#include "CalendarPanel.h"
#include "StatsPanel.h"
#include "StravaClient.h"
#include "WiFiManager.h"

#define USE_SERIAL Serial

// Matrix configuration for MatrixPortal M4 (64x32 RGB matrix)
#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 32
#define MATRIX_CHAIN 1

// Matrix pins for MatrixPortal M4
uint8_t rgbPins[] = {7, 8, 9, 10, 11, 12};
uint8_t addrPins[] = {17, 18, 19, 20};
uint8_t clockPin = 14;
uint8_t latchPin = 15;
uint8_t oePin = 16;

// Create Protomatter matrix object
Adafruit_Protomatter matrix(
    MATRIX_WIDTH, MATRIX_CHAIN,
    1, rgbPins,
    sizeof(addrPins), addrPins,
    clockPin, latchPin, oePin,
    true  // double buffering
);

// Display wrapper
ProtoDisplay display(matrix);

// Button manager
ButtonManager buttons;

// Panel manager
PanelManager panelManager;

// Panels
SplashPanel splashPanel;      // Used for loading only, not in rotation
CalendarPanel calendarPanel;  // Default panel
StatsPanel statsPanel;        // Stats panel

// WiFi manager
WiFiManager wifiManager;

// Strava client
StravaClient* stravaClient = nullptr;

// Activity data storage
CalendarDay activityDays[31];
int activityCount = 0;

// Timing
unsigned long lastFrameTime = 0;
const unsigned long FRAME_INTERVAL_MS = 50;  // ~20 FPS

// API refresh interval (5 minutes)
const unsigned long API_FETCH_INTERVAL_MS = 5 * 60 * 1000;
unsigned long lastApiCallTime = 0;

// Forward declarations
void updateSplashProgress(int percent, const char* status);
void fetchCalendarData();

void setup() {
    USE_SERIAL.begin(115200);
    USE_SERIAL.println("Matrix Miles - Panel System");
    
    // Initialize random seed
    randomSeed(analogRead(0));
    
    // Initialize matrix
    ProtomatterStatus status = matrix.begin();
    USE_SERIAL.printf("Matrix status: %d\n", status);
    
    // Initialize buttons
    buttons.begin();
    USE_SERIAL.println("Buttons initialized (UP=pin2, DOWN=pin3)");
    
    // Setup panel manager (Calendar and Stats only - Splash is for loading)
    panelManager.setDisplay(&display);
    panelManager.addPanel(&calendarPanel);
    panelManager.addPanel(&statsPanel);
    USE_SERIAL.printf("Panels registered: %d\n", panelManager.panelCount());
    
    // Start with splash panel immediately so we can show loading status
    // (Splash is not in PanelManager rotation - just used for loading)
    splashPanel.render(&display);
    
    // Initialize WiFi
    USE_SERIAL.println("Connecting to WiFi...");
    updateSplashProgress(0, "WiFi...");
    
    if (wifiManager.connect(WIFI_SSID, WIFI_PASSWORD)) {
        USE_SERIAL.println("WiFi connected!");
        updateSplashProgress(25, "Connected");
        
        // Initialize Strava client
        stravaClient = new StravaClient(ESP32_API_KEY, SERVER_BASE_URL, USER_ID);
        stravaClient->setUseHTTP(true);
        
        // Sync time from server
        updateSplashProgress(40, "Time...");
        if (stravaClient->syncTimeFromServer()) {
            USE_SERIAL.println("Time synced from server");
            
            // Set calendar to current month and day
            int day, month, year;
            stravaClient->getSyncedDate(day, month, year);
            calendarPanel.setMonth(month, year);
            calendarPanel.setCurrentDay(day);
            statsPanel.setCurrentDate(day, month, year);
        }
        updateSplashProgress(55, "Synced");
        
        // Sync activities with Strava
        updateSplashProgress(70, "Strava...");
        stravaClient->syncActivitiesWithServer();
        
        // Fetch calendar data
        updateSplashProgress(85, "Loading...");
        fetchCalendarData();
        
        updateSplashProgress(100, "Ready!");
        lastApiCallTime = millis();
    } else {
        USE_SERIAL.println("WiFi failed - using demo mode");
        updateSplashProgress(100, "Failed");
    }
    
    // Start panel manager (will show CalendarPanel first)
    panelManager.begin();
    
    lastFrameTime = millis();
    USE_SERIAL.println("Setup complete - entering main loop");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Calculate delta time
    uint32_t deltaMs = currentTime - lastFrameTime;
    
    // Frame rate limiting
    if (deltaMs < FRAME_INTERVAL_MS) {
        return;
    }
    lastFrameTime = currentTime;
    
    // Maintain WiFi connection
    wifiManager.maintain();
    
    // Handle button input
    ButtonEvent event = buttons.update();
    if (event != ButtonEvent::None) {
        USE_SERIAL.printf("Button event: %s\n", 
            event == ButtonEvent::UpPressed ? "UP" : "DOWN");
        panelManager.handleButton(event);
    }
    
    // Periodically refresh calendar data
    if (stravaClient && wifiManager.isConnected()) {
        if (currentTime - lastApiCallTime >= API_FETCH_INTERVAL_MS) {
            fetchCalendarData();
            lastApiCallTime = currentTime;
        }
    }
    
    // Update and render current panel
    panelManager.update(deltaMs);
    panelManager.render();
}

void updateSplashProgress(int percent, const char* status) {
    // Set progress and status on splash panel and render immediately
    splashPanel.setProgress(percent, status);
    splashPanel.render(&display);
}

void fetchCalendarData() {
    if (!stravaClient) return;
    
    int day, month, year;
    stravaClient->getSyncedDate(day, month, year);
    
    if (year <= 1970) {
        USE_SERIAL.println("Invalid date - skipping fetch");
        return;
    }
    
    USE_SERIAL.printf("Fetching calendar for %d/%d\n", month, year);
    
    int count = stravaClient->fetchCalendarData(year, month, activityDays, 31);
    
    if (count > 0) {
        activityCount = count;
        calendarPanel.setMonth(month, year);
        calendarPanel.setActivities(activityDays, count);
        statsPanel.setActivities(activityDays, count);
        USE_SERIAL.printf("Loaded %d activity days\n", count);
    } else {
        USE_SERIAL.println("No activities found");
    }
}
