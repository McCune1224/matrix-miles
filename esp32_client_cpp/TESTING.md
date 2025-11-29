# MatrixPortal M4 Client - Testing & Deployment Guide

## Project Status

✅ **All core features implemented:**
- WiFi connectivity with auto-reconnection
- HTTP/HTTPS API client for Strava calendar data
- Time synchronization from server HTTP headers
- 64x32 RGB matrix display rendering
- Activity calendar visualization
- Comprehensive diagnostics and logging

**Compiled size:** 64,232 bytes (12% of max 507,904 bytes)

## Hardware Setup

1. **Board:** Adafruit MatrixPortal M4 (SAMD51 microcontroller)
2. **WiFi:** Built-in WiFiNINA module
3. **Display:** 64x32 RGB LED matrix (connected via MatrixPortal)
4. **Power:** USB-C for programming and power

## Configuration

Edit `config.hpp` before uploading:

```cpp
#define WIFI_SSID "your_network"
#define WIFI_PASSWORD "your_password"
#define SERVER_BASE_URL "http://your-server-url"  // Use HTTP for embedded systems
#define USER_ID 1                                   // Your Strava user ID
#define ESP32_API_KEY "your-api-key"              // Match server's ESP32_API_KEY
```

## Compilation & Upload

### Prerequisites
```bash
# Install Arduino CLI and board support (see Makefile)
brew install arduino-cli
arduino-cli core install adafruit:samd
```

### Build
```bash
cd esp32_client_cpp
make compile     # Compile the sketch
make upload      # Upload to device (requires PORT=/dev/ttyACM0 for Linux/Mac)
make monitor     # View serial output (115200 baud)
```

## Testing Sequence

### 1. Serial Output Diagnostics
When powered on, the device outputs detailed diagnostics:

```
Matrix Miles - Calendar Display Test
Matrix status: 0
Initializing WiFi...
[WiFi] Connecting to SSID: <your_network>
[WiFi] Connected to: <your_network>
[WiFi] IP Address: 192.168.x.x
[WiFi] Signal Strength (RSSI): -45 dBm

=== Connectivity Tests ===
[Test] ✓ Can reach google.com (internet working)
[Test] Testing API server: http://your-server-url
[Test] ✓ Can reach API server on port 80 (HTTP works)
=== End Connectivity Tests ===

[Strava] === Testing Base URL Connection ===
[Strava] Base URL: http://your-server-url
[Strava] Host: your-server-url
[Strava] Attempting HTTP connection to your-server-url:80
[Strava] ✓ HTTP connection succeeded in XXXms
[Strava] === Sending Health Check Request ===
...

[Strava] === Syncing Time from Server ===
[Strava] Connected to server for time sync
[Strava] Found Date header: Fri, 28 Nov 2025 10:30:45 GMT
[Strava] Parsed time from server: 28/11/2025 10:30:45

Fetching calendar data from API...
Fetching for 11/2025
[Strava] Fetching calendar data: /api/activities/calendar/1/2025/11
[Strava] ✓ Connected in XXXms
[Strava] Bytes sent: XX
[Strava] Response code: 200
[Strava] Response size: XX
[Strava] Parsed X days with activities
Successfully fetched X days with activities
```

### 2. What Each Test Indicates

| Test | Success Indicator | Failure Meaning |
|------|-------------------|-----------------|
| WiFi Connection | SSID connected, IP assigned | Network unreachable or wrong credentials |
| Google.com ping | ✓ status | WiFi connected but no internet |
| API server on port 80 | ✓ status | Server offline, firewall blocking, or wrong URL |
| API server on port 443 | ✓ status | HTTPS available (fallback if HTTP fails) |
| Health endpoint | 200 status code | Server online but API error |
| Date header parsing | Parsed time displayed | Server not returning Date header |
| Calendar API | HTTP 200 + day count | API key invalid, user ID invalid, or DB error |

### 3. Troubleshooting

#### WiFi Won't Connect
- Check SSID and password in config.hpp
- Verify WiFi network is 2.4GHz (NINA doesn't support 5GHz)
- Look for "Connection timeout" in serial output

#### HTTP Connection Fails
- Verify SERVER_BASE_URL format: `http://hostname` or `http://ip:port`
- Check server is running: `curl http://your-server/health`
- Verify firewall isn't blocking port 80
- Try HTTPS as fallback (use `https://` in URL)

#### Calendar Data Not Fetching
- Verify API key matches server's `ESP32_API_KEY` environment variable
- Check user ID exists in database: `SELECT * FROM users WHERE id = 1;`
- Confirm there are activities in the database for the requested month
- Check server logs for API errors

#### Time Not Syncing
- Server must return `Date:` header in HTTP response
- Check with: `curl -i http://your-server/health | grep Date`
- Device time is currently not actually set (TODO for future)

#### Display Issues
- Check matrix is powered separately (64x32 draws ~2-4A)
- Look for matrix initialization status in serial output
- Verify matrix is connected to correct pins

## Production Deployment

### Server Requirements

The Go backend must:
1. Expose `/health` endpoint (returns 200 with Date header)
2. Expose `/api/activities/calendar/:userId/:year/:month` endpoint
3. Validate `X-API-Key` header
4. Respond with JSON array in format:
   ```json
   [
     {"activity_date": "2025-11-01", "count": 1, "total_distance": "5.5"},
     {"activity_date": "2025-11-15", "count": 2, "total_distance": "12.3"}
   ]
   ```

### HTTP vs HTTPS
- **Preferred:** HTTP on port 80 (simpler for embedded systems)
- **Alternative:** HTTPS on port 443 (use BearSSL if certificates fail)
- **Current limitation:** WiFiNINA certificate validation can be strict

To test HTTPS, modify code to use `connectSSL()`:
```cpp
stravaClient->setUseHTTP(false);  // Use HTTPS instead
```

### Update Interval
- Calendar data fetches every 5 minutes (configurable)
- Adjust in main sketch: `API_FETCH_INTERVAL_MS`

## Known Limitations

1. **No Real RTC:** Device has no battery-backed clock. Time resets on power loss.
   - Currently: Synced from server on startup (parsed but not persisted)
   - Future: Use SAMD51 RTC peripheral with external crystal

2. **Certificate Validation:** WiFiNINA can't validate some certificates
   - Workaround: Use HTTP instead
   - Alternative: Use BearSSL with `connectBearSSL()`

3. **Matrix Display:** Fixed to November 2025 in current code
   - Should use synced time to determine current month
   - TODO: Implement month cycling with button controls

4. **Memory:** Device has limited RAM (256KB)
   - Calendar display limited to 31 days
   - JSON parsing limited to ~8KB responses

## Future Enhancements

- [ ] Button controls for month navigation
- [ ] RTC implementation for persistent time
- [ ] Multiple activity display modes (distance, count, etc.)
- [ ] EEPROM storage for credentials
- [ ] OTA firmware updates
- [ ] Bluetooth configuration portal
- [ ] Animation effects between updates

## Files Modified

- **esp32_client_cpp.ino** - Main sketch with WiFi init and API calls
- **StravaClient.h/cpp** - HTTP API client with time sync
- **WiFiManager.h/cpp** - WiFi connectivity management
- **MatrixDisplay.h/cpp** - Calendar rendering
- **config.hpp** - Configuration constants
- **.clangd** - IDE configuration for proper code intelligence

## Support

For issues, check:
1. Serial output for detailed error messages
2. Server logs for API-side errors
3. Network connectivity with `testConnectivity()` function
4. Device firmware version (Arduino 1.8.16 recommended)
