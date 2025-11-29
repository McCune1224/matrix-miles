// WiFi Configuration
// DO NOT commit this file to git!

#ifndef CONFIG_HPP
#define CONFIG_HPP

// WiFi Credentials - Replace with your actual WiFi network details
const char* WIFI_SSID = "black_mesa";
const char* WIFI_PASSWORD = "thecakeisalie!";

// API Configuration - Replace with your actual API key
#define ESP32_API_KEY "9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6"

// Server Configuration - Base URL (no trailing slash)
#define USE_PRODUCTION true

#if USE_PRODUCTION
const char* SERVER_BASE_URL = "https://matrix-miles-production.up.railway.app/api";
#else
const char* SERVER_BASE_URL = "http://your-test-server.com";
#endif

// User ID for API requests
const int USER_ID = 1;

#endif // CONFIG_HPP
