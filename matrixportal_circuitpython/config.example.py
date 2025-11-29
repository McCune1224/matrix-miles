"""
Configuration template for Matrix Miles CircuitPython

Copy this file to secrets.py and fill in your actual values.
The secrets.py file is in .gitignore and should NOT be committed.
"""

# ============================================================================
# WiFi Configuration
# ============================================================================
WIFI_SSID = "your-wifi-network"
WIFI_PASSWORD = "your-wifi-password"

# ============================================================================
# API Configuration
# ============================================================================

# Your Matrix Miles backend base URL
# For production: https://matrix-miles-production.up.railway.app
# For local development: http://192.168.1.100:8080 (if no HTTPS)
API_BASE_URL = "https://matrix-miles-production.up.railway.app"

# API key from your backend (generate in admin panel)
API_KEY = "your-api-key-here"

# User ID to fetch activities for
USER_ID = 1

# ============================================================================
# Display Configuration
# ============================================================================

# How often to fetch new data (in seconds)
# 300 = 5 minutes (recommended to avoid rate limiting)
# 60 = 1 minute (for testing)
REFRESH_INTERVAL_SECONDS = 300

# Display brightness (0.0 to 1.0)
DISPLAY_BRIGHTNESS = 1.0

# ============================================================================
# Optional: Advanced Configuration
# ============================================================================

# WiFi connection timeout (seconds)
WIFI_TIMEOUT = 30

# API request timeout (seconds)
API_TIMEOUT = 10

# Enable debug logging to serial
DEBUG_MODE = True
