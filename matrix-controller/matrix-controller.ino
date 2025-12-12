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
void showLoadingText(const char* text, int progress = -1);

void setup() {
    USE_SERIAL.begin(115200);
    USE_SERIAL.println("Matrix Miles - Panel System");
    
    // Initialize random seed
    randomSeed(analogRead(0));
    
    // Initialize matrix
    ProtomatterStatus status = matrix.begin();
    USE_SERIAL.printf("Matrix status: %d\n", status);
    
    // Give matrix time to initialize
    delay(100);
    
    // Clear screen during loading (no flashbang)
    display.fillScreen(0);
    display.show();
    
    // Initialize buttons
    buttons.begin();
    USE_SERIAL.println("Buttons initialized (UP=pin2, DOWN=pin3)");
    
    // Check WiFi module
    if (WiFi.status() == WL_NO_MODULE) {
        USE_SERIAL.println("ERROR: WiFi module not found!");
        showLoadingText("No WiFi!");
        while (true) { delay(1000); }  // Halt
    }
    
    String fv = WiFi.firmwareVersion();
    USE_SERIAL.print("WiFi firmware: ");
    USE_SERIAL.println(fv);
    
    // Setup panel manager
    panelManager.setDisplay(&display);
    panelManager.addPanel(&calendarPanel);
    panelManager.addPanel(&weatherPanel);
    USE_SERIAL.printf("Panels registered: %d\n", panelManager.panelCount());
    
    // Initialize WiFi
    USE_SERIAL.println("Connecting to WiFi...");
    showLoadingText("", 0);
    
    if (wifiManager.connect(WIFI_SSID, WIFI_PASSWORD, 20000)) {  // 20 second timeout
        USE_SERIAL.println("WiFi connected!");
        showLoadingText("", 20);
        
        // Initialize Strava client
        stravaClient = new StravaClient(ESP32_API_KEY, SERVER_BASE_URL, USER_ID);
        stravaClient->setUseHTTP(true);
        
        // Sync time from server
        showLoadingText("", 35);
        if (stravaClient->syncTimeFromServer()) {
            USE_SERIAL.println("Time synced from server");
            
            // Set calendar to current month and day
            int day, month, year;
            stravaClient->getSyncedDate(day, month, year);
            calendarPanel.setMonth(month, year);
            calendarPanel.setCurrentDay(day);
        }
        
        // Sync activities with Strava
        showLoadingText("", 50);
        stravaClient->syncActivitiesWithServer();
        
        // Fetch calendar data
        showLoadingText("", 70);
        fetchCalendarData();
        
        // Fetch weather data
        showLoadingText("", 90);
        fetchWeatherData();
        
        showLoadingText("", 100);
        lastApiCallTime = millis();
    } else {
        USE_SERIAL.println("WiFi failed - using demo mode");
        // Show empty bar on failure
        showLoadingText("", 0);
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

// Display loading text on matrix during startup with progress bar
void showLoadingText(const char* text, int progress) {
    display.fillScreen(0);
    
    // Progress bar with white border
    int barWidth = 50;
    int barHeight = 6;
    int barX = (64 - barWidth) / 2;  // Center horizontally
    int barY = 8;
    
    // White border (1px)
    uint16_t white = display.color565(180, 180, 180);
    display.drawFastHLine(barX, barY, barWidth, white);                    // Top
    display.drawFastHLine(barX, barY + barHeight - 1, barWidth, white);    // Bottom
    display.drawFastVLine(barX, barY, barHeight, white);                   // Left
    display.drawFastVLine(barX + barWidth - 1, barY, barHeight, white);    // Right
    
    // Filled portion (green) - inside the border
    if (progress > 0) {
        int innerWidth = barWidth - 2;
        int filledWidth = (progress * innerWidth) / 100;
        if (filledWidth > 0) {
            display.fillRect(barX + 1, barY + 1, filledWidth, barHeight - 2, display.color565(0, 200, 0));
        }
    }
    
    // "Loading..." text (centered below bar)
    display.setTextSize(1);
    display.setTextColor(display.color565(180, 180, 180));
    const char* loadingText = "Loading...";
    int textWidth = strlen(loadingText) * 6;
    int textX = (64 - textWidth) / 2;
    display.setCursor(textX, 18);
    display.print(loadingText);
    
    display.show();
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
