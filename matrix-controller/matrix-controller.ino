#include <Arduino.h>
#include <Adafruit_Protomatter.h>
#include <ctime>

#include "config.hpp"
#include "ProtoDisplay.h"
#include "ButtonManager.h"
#include "PanelManager.h"
#include "Transition.h"
#include "CalendarPanel.h"
#include "WeatherPanel.h"
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
CalendarPanel calendarPanel;  // Default panel
WeatherPanel weatherPanel;    // Weather panel

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
void fetchCalendarData();
void fetchWeatherData();
void showLoadingText(const char* text);

void setup() {
    USE_SERIAL.begin(115200);
    USE_SERIAL.println("Matrix Miles - Panel System");
    
    // Initialize random seed
    randomSeed(analogRead(0));
    
    // Initialize matrix
    ProtomatterStatus status = matrix.begin();
    USE_SERIAL.printf("Matrix status: %d\n", status);
    
    // Clear screen during loading (no flashbang)
    display.fillScreen(0);
    display.show();
    
    // Initialize buttons
    buttons.begin();
    USE_SERIAL.println("Buttons initialized (UP=pin2, DOWN=pin3)");
    
    // Setup panel manager
    panelManager.setDisplay(&display);
    panelManager.addPanel(&calendarPanel);
    panelManager.addPanel(&weatherPanel);
    USE_SERIAL.printf("Panels registered: %d\n", panelManager.panelCount());
    
    // Initialize WiFi
    USE_SERIAL.println("Connecting to WiFi...");
    showLoadingText("WiFi...");
    
    if (wifiManager.connect(WIFI_SSID, WIFI_PASSWORD)) {
        USE_SERIAL.println("WiFi connected!");
        
        // Initialize Strava client
        stravaClient = new StravaClient(ESP32_API_KEY, SERVER_BASE_URL, USER_ID);
        stravaClient->setUseHTTP(true);
        
        // Sync time from server
        showLoadingText("Syncing...");
        if (stravaClient->syncTimeFromServer()) {
            USE_SERIAL.println("Time synced from server");
            
            // Set calendar to current month and day
            int day, month, year;
            stravaClient->getSyncedDate(day, month, year);
            calendarPanel.setMonth(month, year);
            calendarPanel.setCurrentDay(day);
        }
        
        // Sync activities with Strava
        showLoadingText("Loading...");
        stravaClient->syncActivitiesWithServer();
        
        // Fetch calendar data
        fetchCalendarData();
        
        // Fetch weather data
        fetchWeatherData();
        
        lastApiCallTime = millis();
    } else {
        USE_SERIAL.println("WiFi failed - using demo mode");
    }
    
    // Start panel manager with ConcentricShapes transition to CalendarPanel
    panelManager.beginWithTransition(TransitionType::ConcentricShapes, 500);
    
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
    
    // Periodically refresh data
    if (stravaClient && wifiManager.isConnected()) {
        if (currentTime - lastApiCallTime >= API_FETCH_INTERVAL_MS) {
            fetchCalendarData();
            fetchWeatherData();
            lastApiCallTime = currentTime;
        }
    }
    
    // Update and render current panel
    panelManager.update(deltaMs);
    panelManager.render();
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
        USE_SERIAL.printf("Loaded %d activity days\n", count);
    } else {
        USE_SERIAL.println("No activities found");
    }
}

// Display loading text on matrix during startup
void showLoadingText(const char* text) {
    display.fillScreen(0);
    display.setTextSize(1);
    display.setTextColor(display.color565(100, 100, 100));  // Dim gray
    display.setCursor(2, 12);
    display.print(text);
    display.show();
    delay(50);  // Small delay to ensure display refreshes
}

// Map condition string to WeatherCondition enum
WeatherCondition mapConditionString(const char* condition) {
    if (strcmp(condition, "sunny") == 0) return WeatherCondition::Sunny;
    if (strcmp(condition, "partly_cloudy") == 0) return WeatherCondition::PartlyCloudy;
    if (strcmp(condition, "cloudy") == 0) return WeatherCondition::Cloudy;
    if (strcmp(condition, "rainy") == 0) return WeatherCondition::Rainy;
    if (strcmp(condition, "snowy") == 0) return WeatherCondition::Snowy;
    if (strcmp(condition, "windy") == 0) return WeatherCondition::Windy;
    if (strcmp(condition, "stormy") == 0) return WeatherCondition::Stormy;
    return WeatherCondition::Unknown;
}

void fetchWeatherData() {
    if (!stravaClient) return;
    
    USE_SERIAL.println("Fetching weather data...");
    
    WeatherData weather;
    if (stravaClient->fetchWeather(&weather)) {
        WeatherCondition condition = mapConditionString(weather.condition);
        weatherPanel.setWeather(condition, weather.tempF, weather.humidity);
        USE_SERIAL.printf("Weather updated: %s, %dF, %d%%\n", 
                          weather.condition, weather.tempF, weather.humidity);
    } else {
        USE_SERIAL.println("Failed to fetch weather - using defaults");
        weatherPanel.setWeather(WeatherCondition::Unknown, 0, 0);
    }
}
