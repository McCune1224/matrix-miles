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

// SSL Certificate Fingerprint - Railway.app SHA256 (updated: Nov 2025)
// Extract with: echo | openssl s_client -connect matrix-miles-production.up.railway.app:443 -servername matrix-miles-production.up.railway.app 2>/dev/null | openssl x509 -noout -fingerprint -sha256 | sed 's/SHA256 Fingerprint=//' | tr -d ':'
const char* RAILWAY_CERT_SHA256 = "4EE4ADB2CFF9E47C44B4A72FC2C134584C225CA04FFAC28EDE02776367F61CF1";

// User ID for API requests
const int USER_ID = 1;

#endif // CONFIG_HPP
