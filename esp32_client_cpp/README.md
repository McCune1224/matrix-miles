# ESP32 Strava Activity Fetcher

Fetches Strava activities from the server every 5 minutes and outputs JSON to serial.

## Tech Stack

- ESP32 microcontroller
- Arduino C++
- HTTPClient for REST API calls
- ArduinoJson for parsing (optional)
- WiFi connection via WiFiMulti

## Setup

### 1. Configure WiFi Credentials

Copy the example config and add your WiFi details:

```bash
cd esp32_client_cpp/blink
cp config.h.example config.h
nano config.h
```

Update these values in `config.h`:

```cpp
const char* WIFI_SSID = "your_wifi_network_name";
const char* WIFI_PASSWORD = "your_wifi_password";
#define ESP32_API_KEY "your_api_key_here"
```

### 2. Install Required Libraries

Install via Arduino IDE Library Manager:

- ArduinoJson (version 6.x)

### 3. Upload to ESP32

1. Open `blink.ino` in Arduino IDE
2. Select ESP32 board (Tools → Board → ESP32 Dev Module)
3. Select COM port (Tools → Port)
4. Click Upload

### 4. Monitor Output

Open Serial Monitor at 115200 baud rate.

## Output

Raw JSON is printed to serial every 5 minutes:

```
[HTTP] GET https://matrix-miles-production.up.railway.app/api/activities/recent/1
[HTTP] Response code: 200

========== ACTIVITIES JSON ==========
[{"id":1,"user_id":1,"strava_activity_id":12345,"name":"Morning Run"...}]
=====================================
```

Optional: Uncomment the JSON parsing section in `blink.ino` (lines 63-85) for formatted output.

## Configuration

### Fetch Interval

Edit `FETCH_INTERVAL_MS` in `blink.ino` (default: 5 minutes):

```cpp
const unsigned long FETCH_INTERVAL_MS = 300000;
```

### User ID

Change `USER_ID` in `config.h` to fetch activities for different users:

```cpp
const int USER_ID = 2;
```

### Server Environment

Switch between production and test servers in `config.h`:

```cpp
#define USE_PRODUCTION false
```

## Security

Do not commit `config.h` - it contains WiFi credentials and API keys. The file is already in `.gitignore`.

## Troubleshooting

**WiFi connection fails:** Check SSID/password, ensure 2.4GHz network (ESP32 doesn't support 5GHz)

**HTTP 401:** Verify API key matches server configuration

**HTTP 404:** Check user ID exists in database

**JSON parse error:** Install ArduinoJson 6.x, increase `DynamicJsonDocument` size if needed

## API

URLs are built from `SERVER_BASE_URL` and `USER_ID`:

```
GET {SERVER_BASE_URL}/api/activities/recent/{USER_ID}
Headers: X-API-Key: <your-api-key>
```

Add new endpoints by creating functions in `blink.ino`:

```cpp
void fetchCalendarData(int year, int month) {
  String url = String(SERVER_BASE_URL) + "/api/activities/calendar/" + 
               String(USER_ID) + "/" + String(year) + "/" + String(month);
  // HTTP request code here
}
```
