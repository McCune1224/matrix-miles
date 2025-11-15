# ESP32 Strava Activity Fetcher

This ESP32 client fetches recent Strava activities from the production server every 5 minutes and outputs them as JSON to the serial monitor.

## Setup Instructions

### 1. Configure WiFi Credentials (First Time Setup)

The project uses a `config.h` file to store sensitive credentials that should **NOT** be committed to git.

```bash
# Navigate to the blink directory
cd esp32_client_cpp/blink

# Copy the example config file
cp config.h.example config.h

# Edit config.h with your actual credentials
nano config.h
```

In `config.h`, update these values:

```cpp
// Replace with your WiFi network name
const char* WIFI_SSID = "your_wifi_network_name";

// Replace with your WiFi password
const char* WIFI_PASSWORD = "your_wifi_password";

// The API key is already configured for production
#define ESP32_API_KEY "9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6"
```

### 2. Install Required Libraries

Open Arduino IDE and install these libraries via Library Manager (Sketch → Include Library → Manage Libraries):

- **ArduinoJson** (version 6.x) - For JSON parsing

### 3. Upload to ESP32

1. Open `blink.ino` in Arduino IDE
2. Select your ESP32 board (Tools → Board → ESP32 Dev Module)
3. Select the correct COM port (Tools → Port)
4. Click Upload

### 4. Monitor Output

Open Serial Monitor (Tools → Serial Monitor) and set baud rate to **115200**.

## Features

- ✅ Fetches activities every 5 minutes
- ✅ Outputs raw JSON to serial monitor
- ✅ Non-blocking loop using `millis()`
- ✅ Automatic WiFi reconnection
- ✅ Configurable production/test server endpoints
- ✅ Optional formatted JSON parsing

## Expected Output

```
=================================
Strava Activity Fetcher for ESP32
=================================
[SETUP] WAIT 4...
[SETUP] WAIT 3...
[SETUP] WAIT 2...
[SETUP] WAIT 1...
[SETUP] Connecting to WiFi...

[HTTP] Fetching recent activities...
[HTTP] GET https://matrix-miles-production.up.railway.app/api/activities/recent/1
[HTTP] Response code: 200

========== ACTIVITIES JSON ==========
[{"id":1,"user_id":1,"strava_activity_id":12345,"name":"Morning Run","type":"Run","distance":5000.0,"moving_time":1800,"start_date":"2025-11-14T08:00:00Z"}]
=====================================

[INFO] Next fetch in 300 seconds (5 minutes)
```

## Optional: Enable Formatted JSON Output

To get nicely formatted activity information instead of raw JSON:

1. Make sure ArduinoJson library is installed
2. Open `blink.ino`
3. Uncomment lines 76-95 (the JSON parsing section)
4. Re-upload to ESP32

You'll then see output like:

```
Found 3 activities:

Activity #1:
  Name: Morning Run
  Type: Run
  Distance: 5.00 km
  Moving Time: 1800 seconds
  Start Date: 2025-11-14T08:00:00Z

Activity #2:
  Name: Evening Ride
  Type: Ride
  Distance: 15.50 km
  Moving Time: 2700 seconds
  Start Date: 2025-11-13T18:00:00Z
```

## Configuration Options

### Change Fetch Interval

Edit `FETCH_INTERVAL_MS` in `blink.ino`:

```cpp
const unsigned long FETCH_INTERVAL_MS = 300000;  // 5 minutes in milliseconds
```

Common intervals:
- 1 minute: `60000`
- 5 minutes: `300000`
- 10 minutes: `600000`
- 15 minutes: `900000`

### Change User ID

Edit `config.h` to fetch activities for a different user:

```cpp
const int USER_ID = 2;  // Change to desired user ID
```

### Switch to Test Server

Edit `config.h`:

```cpp
#define USE_PRODUCTION false  // Change true to false
```

The code will automatically build URLs like:
- Production: `https://matrix-miles-production.up.railway.app/api/activities/recent/1`
- Test: `https://your-test-server.com/api/activities/recent/1`

## Security Notes

- ⚠️ **NEVER commit `config.h`** - It contains your WiFi credentials
- ✅ `config.h` is already in `.gitignore`
- ✅ Only commit `config.h.example` as a template
- ✅ Share `config.h.example` with team members, not `config.h`

## Troubleshooting

### WiFi Connection Failed

- Double-check SSID and password in `config.h`
- Ensure WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
- Check that WiFi is in range

### HTTP Error 401 (Unauthorized)

- Verify API key in `config.h` matches server configuration
- Check that server is running

### HTTP Error 404 (Not Found)

- Verify user ID exists in database (currently hardcoded to user ID 1)
- Check server URL is correct

### JSON Parse Error

- Ensure ArduinoJson library is installed
- Verify you're using ArduinoJson version 6.x (not 5.x or 7.x)
- Increase `DynamicJsonDocument` size if activities are very large

## Project Structure

```
esp32_client_cpp/blink/
├── blink.ino           # Main ESP32 code
├── config.h            # WiFi credentials (NEVER commit)
├── config.h.example    # Template for config.h (commit this)
├── sketch.yaml         # Arduino CLI configuration
├── Makefile            # Build automation
└── README.md           # This file
```

## API Endpoint

The ESP32 builds URLs dynamically from the base URL and user ID:
```
GET {SERVER_BASE_URL}/api/activities/recent/{USER_ID}
Headers: X-API-Key: <your-api-key>
```

Example:
```
GET https://matrix-miles-production.up.railway.app/api/activities/recent/1
```

### Adding New Endpoints

To fetch different data, add new functions to `blink.ino`:

```cpp
void fetchCalendarData(int year, int month) {
  String url = String(SERVER_BASE_URL) + "/api/activities/calendar/" + 
               String(USER_ID) + "/" + String(year) + "/" + String(month);
  // ... rest of HTTP code
}

void fetchUserStats() {
  String url = String(SERVER_BASE_URL) + "/api/stats/" + String(USER_ID);
  // ... rest of HTTP code
}
```

## License

MIT
