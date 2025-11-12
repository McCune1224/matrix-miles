# config.py - ESP32 Configuration

# WiFi credentials
WIFI_SSID = "YOUR_WIFI_NAME"
WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"

# Server configuration
# Use your Railway server URL or ngrok tunnel for testing
# For local testing: 'http://192.168.1.XXX:8080' (your machine's IP)
API_BASE_URL = "http://localhost:8080"  # UPDATE THIS!

# Your ESP32 API key (from strava-server/.env)
API_KEY = "9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6"

# Your Strava user ID (get this from OAuth flow)
USER_ID = 1  # UPDATE THIS after OAuth!

# How often to refresh data (seconds)
REFRESH_INTERVAL_SECONDS = 300  # 5 minutes
