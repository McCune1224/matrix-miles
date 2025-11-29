# MatrixPortal M4 - CircuitPython Edition

A clean, maintainable CircuitPython implementation for your MatrixPortal M4 that fetches Strava activities from your Go backend via HTTPS and displays them on the LED matrix as a calendar view.

## Why CircuitPython Over C++?

| Aspect | C++ (Arduino) | CircuitPython |
|--------|---------------|---------------|
| **HTTPS/SSL** | Complex certificate handling | Native, built-in SSL support |
| **Development** | Compile + upload cycle | Live REPL, instant changes |
| **JSON Parsing** | Manual or ArduinoJson library | Native dict/list support |
| **Code Simplicity** | ~400+ lines for basic HTTP | ~100 lines for full stack |
| **WiFi Reconnection** | Manual implementation | Built-in with simpler logic |
| **Matrix Display** | Limited libraries | Rich Adafruit ecosystem |

## Project Overview

This application:
1. Connects to WiFi
2. Makes HTTPS requests to your Matrix Miles backend API
3. Parses JSON activity data
4. Renders a calendar view on the LED matrix showing activity dates
5. Handles WiFi reconnections gracefully
6. Runs continuously with configurable refresh intervals

## Architecture

```
MatrixPortal M4 (CircuitPython)
├── code.py (main loop)
│   ├── wifi_manager.py (WiFi + reconnection)
│   ├── api_client.py (HTTPS requests)
│   └── calendar_display.py (LED matrix rendering)
└── secrets.py (credentials - NOT committed)
```

## Quick Start

### Prerequisites
- MatrixPortal M4 running CircuitPython 8.x or later
- Your Matrix Miles backend running and accessible
- API key from your backend admin

### Setup (5 minutes)

1. **Install CircuitPython** on your MatrixPortal M4
   ```bash
   # See GETTING_STARTED.md for detailed steps
   ```

2. **Configure credentials**
   ```bash
   cp config.example.py secrets.py
   # Edit secrets.py with your WiFi and API details
   ```

3. **Copy files to device**
   ```bash
   # Copy code.py and lib/ folder to CIRCUITPY drive
   cp code.py /Volumes/CIRCUITPY/
   cp -r lib /Volumes/CIRCUITPY/
   cp secrets.py /Volumes/CIRCUITPY/
   ```

4. **Monitor with serial connection**
   ```bash
   # Open serial monitor at 115200 baud to see debug output
   ```

## File Structure

- **code.py** - Main application entry point and event loop
- **config.example.py** - Configuration template (copy to secrets.py)
- **lib/wifi_manager.py** - Handles WiFi connection and reconnection
- **lib/api_client.py** - HTTPS API requests with error handling
- **lib/calendar_display.py** - Renders activity data on LED matrix
- **docs/API_ENDPOINTS.md** - Reference for your backend API

## Configuration

See `config.example.py` for all available options:

```python
# WiFi
WIFI_SSID = "your-network"
WIFI_PASSWORD = "password"

# API
API_BASE_URL = "https://matrix-miles-production.up.railway.app"
API_KEY = "your-api-key"
USER_ID = 1

# Display
REFRESH_INTERVAL_SECONDS = 300  # Fetch every 5 minutes
DISPLAY_BRIGHTNESS = 1.0
```

## API Integration

Your backend provides these endpoints (see docs/API_ENDPOINTS.md):

```
GET /api/activities/recent/{USER_ID}
GET /api/activities/calendar/{USER_ID}/{YEAR}/{MONTH}
```

The CircuitPython client handles:
- SSL/TLS certificate validation
- Automatic retries on network failures
- Graceful error handling
- Structured logging to serial

## Debugging

Monitor the serial output (115200 baud) to see:
- WiFi connection status
- API request/response logs
- JSON parsing results
- Display rendering updates
- Error messages with context

## Architecture Details

### WiFi Management
- Non-blocking connection attempts with timeout
- Automatic reconnection on disconnect
- Signal strength monitoring

### API Client
- Standard HTTP headers (Content-Type, User-Agent)
- API key authentication via X-API-Key header
- JSON response parsing
- HTTP error code handling
- Automatic retry logic (optional)

### Display Rendering
- Calendar grid layout (7 columns for days of week)
- Color-coded activity indicators
- Activity type icons (run, bike, etc.)
- Date labels and statistics

## Troubleshooting

See GETTING_STARTED.md for common issues and solutions.

## Next Steps

After basic setup:
1. Customize calendar colors and display format
2. Add activity type filtering
3. Implement activity statistics display
4. Add push-button controls for navigation
5. Optimize power consumption with deep sleep

## License

MIT (same as Matrix Miles)
