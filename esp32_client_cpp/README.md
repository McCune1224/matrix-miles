# MatrixPortal M4 Strava Activity Fetcher

Fetches Strava activities from the server every 5 minutes and outputs JSON to serial.

## Tech Stack

- **Adafruit MatrixPortal M4** microcontroller with integrated ESP32 WiFi co-processor
- **Arduino C++** with ESP32 core
- **Adafruit_MQTT** or **HTTPClient** for REST API calls (HTTPClient for HTTPS with TLS)
- **ArduinoJson** (v6.x or later) for JSON parsing
- **WiFi** + **WiFiMulti** (ESP32 native) for robust WiFi connectivity

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

Install via Arduino IDE Library Manager (**Sketch → Include Library → Manage Libraries**):

**Required:**
- **ArduinoJson** (v6.x or later) - JSON parsing
- **HTTPClient** (included with ESP32 core) - HTTPS requests with TLS/SSL support

**Recommended (Adafruit ecosystem):**
- **Adafruit Protomatter** - RGB matrix display control
- **Adafruit_Sensor** - Sensor abstraction layer
- **Adafruit_LIS3DH** - Accelerometer (optional, for gesture detection)

**Note:** The ESP32 core includes WiFi, WiFiMulti, and HTTPClient libraries natively. HTTPClient supports HTTPS with certificate validation for secure API communication.

### 3. Upload to ESP32

1. Open `blink.ino` in Arduino IDE
2. Select Adafruit MatrixPortal M4 board (Tools → Board → Adafruit MatrixPortal M4)
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

## WiFi & Networking Best Practices

### Library Selection

For the MatrixPortal M4, use **ESP32 native libraries** (included with Arduino ESP32 core):

- **WiFi.h** + **WiFiMulti.h** - Robust connection management
- **HTTPClient.h** - Built-in HTTPS/TLS support (preferred for API calls)

### WiFi Connection Pattern

```cpp
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

WiFiMulti wifiMulti;

void setup() {
  // Add multiple APs for fallback
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  
  // Non-blocking connection with timeout
  int attempts = 0;
  while (wifiMulti.run() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
  }
  
  if (WiFi.isConnected()) {
    Serial.print("Connected to: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  // Reconnect if disconnected
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi disconnected!");
    return;
  }
  
  // Make HTTPS request
  HTTPClient http;
  http.begin(url);
  http.addHeader("X-API-Key", ESP32_API_KEY);
  
  int responseCode = http.GET();
  if (responseCode == 200) {
    String payload = http.getString();
    // Process payload
  }
  http.end();
}
```

### TLS/Certificate Handling

HTTPClient supports certificate validation for HTTPS. For production use:

```cpp
// With certificate fingerprint (for Railway or other providers)
http.begin(url, "AA:BB:CC:DD:EE:FF:00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD");

// Or skip validation (NOT recommended for production)
// http.setInsecure();
```



## Troubleshooting

**WiFi connection fails:** 
- Check SSID/password are correct
- Ensure 2.4GHz network (MatrixPortal M4 ESP32 doesn't support 5GHz)
- Use WiFiMulti for fallback AP support
- Verify WiFi signal strength (RSSI) with `WiFi.RSSI()`

**HTTPS/TLS certificate errors:** 
- HTTPClient supports TLS/SSL with certificate validation
- For production servers, ensure certificates are valid
- Use `https://` URLs with HTTPClient for encrypted communication

**HTTP 401 (Unauthorized):** 
- Verify API key matches server configuration
- Confirm custom headers are being sent correctly with `setAuthorization()` or custom header methods

**HTTP 404 (Not Found):** 
- Check user ID exists in database
- Verify URL formatting and endpoint paths

**JSON parse error:** 
- Install ArduinoJson 6.x (or later)
- Increase `DynamicJsonDocument` size if response is large
- Check response is valid JSON with serial debugging

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
