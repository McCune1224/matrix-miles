# HTTPS Debugging Guide

## Quick Reference

| Issue | Likely Cause | Solution |
|-------|-------------|----------|
| "HTTPS connection failed" | Certificate not trusted | Check WiFi, internet, server status |
| "HTTP fallback succeeded" | HTTPS unavailable but HTTP works | Normal - device will retry HTTPS next cycle |
| "Connection failed after 8234ms" | Network timeout | Check WiFi signal, firewall, internet |
| Response code 401 | API key invalid | Verify API_KEY in config.hpp |
| Response code 404 | Wrong endpoint | Check SERVER_BASE_URL and USER_ID |
| "Parsed 0 days" | JSON parsing failed | Check response format on server |

---

## Serial Output Analysis

### Reading Serial Monitor

**Port:** /dev/ttyACM0 (or /dev/cu.usbmodem* on macOS)
**Baud:** 115200
**Start monitor:**
```bash
cd esp32_client_cpp && make monitor PORT=/dev/ttyACM0
```

### Key Log Lines to Look For

#### WiFi Connection
```
✓ GOOD: [WiFi] ✓ Connected! SSID: black_mesa, Signal: -45 dBm
✗ BAD:  [WiFi] Connection failed after 30000ms (timeout)
```

#### HTTPS Connection
```
✓ GOOD: [Strava] ✓ HTTPS connection succeeded in 245ms
✗ BAD:  [Strava] ✗ HTTPS connection failed after 123ms
```

#### HTTP Fallback
```
✓ OK:   [Strava] ✓ HTTP fallback succeeded in 89ms
✗ BAD:  [Strava] HTTP fallback also failed
```

#### Server Response
```
✓ GOOD: [Strava] Response code: 200
✗ BAD:  [Strava] Response code: 401  # Authentication failed
✗ BAD:  [Strava] Response code: 404  # Not found
✗ BAD:  [Strava] Response code: 500  # Server error
```

#### Data Parsing
```
✓ GOOD: [Strava] Parsed 8 days with activities
✗ BAD:  [Strava] Parsed 0 days with activities
✗ BAD:  [Strava] JSON parse error
```

---

## Common Issues & Solutions

### Issue 1: Device Stuck on WiFi Connection

**Serial Output:**
```
Initializing WiFi...
[WiFi] Connecting to black_mesa...
[WiFi] Waiting for connection... (repeats)
```

**Root Causes:**
1. WiFi credentials wrong
2. Router not broadcasting SSID
3. Too far from router

**Solution:**
1. Verify WiFi SSID and password in config.hpp
2. Check router is on and broadcasting "black_mesa"
3. Move device closer to router
4. Check WiFi signal: Look for "Signal: -XX dBm" (should be -30 to -70, closer to 0 is better)

**Debug:**
```
Expected: [WiFi] ✓ Connected! SSID: black_mesa, Signal: -45 dBm
Actual:   [WiFi] Connection failed after 30000ms
```

---

### Issue 2: WiFi Connected but No Internet

**Serial Output:**
```
[WiFi] ✓ Connected! SSID: black_mesa, Signal: -45 dBm
[Test] ✗ Cannot reach google.com (internet may be down)
```

**Root Causes:**
1. Router has no internet
2. Firewall blocking device
3. Device IP not on correct subnet

**Solution:**
1. Check router has internet: Is it connected to modem?
2. Check device IP: Look for line like `[WiFi] IP: 192.168.1.XX`
3. Restart router and device

**Debug:**
```bash
# From computer on same WiFi:
ping matrix-miles-production.up.railway.app
# Should get responses

nslookup matrix-miles-production.up.railway.app
# Should resolve to IP address
```

---

### Issue 3: Cannot Reach API Server

**Serial Output:**
```
[Test] ✓ Can reach google.com (internet working)
[Test] ✗ Cannot reach API server on 443 (HTTPS)
[Test] ✗ Cannot reach API server on port 80 either
```

**Root Causes:**
1. Server is down
2. Railway deployment not running
3. Firewall blocking ports 443 and 80
4. DNS not resolving domain

**Solution:**
1. Check Railway dashboard - is deployment running?
2. Test from computer: `https://matrix-miles-production.up.railway.app/health`
3. Check if ISP blocks port 443 or 80
4. Try DNS lookup: `nslookup matrix-miles-production.up.railway.app`

**Debug:**
```bash
# From computer:
curl -v https://matrix-miles-production.up.railway.app/health
# Should return 200 OK

openssl s_client -connect matrix-miles-production.up.railway.app:443 -servername matrix-miles-production.up.railway.app
# Should show certificate info (quit with Ctrl-C)
```

---

### Issue 4: HTTPS Connection Fails, HTTP Falls Back

**Serial Output:**
```
[Strava] ✗ HTTPS connection failed after 123ms
[Strava] → Attempting HTTP fallback...
[Strava] ✓ HTTP fallback succeeded in 89ms
```

**Root Causes:**
1. Certificate validation failed
2. TLS handshake failed
3. Device time is wrong (certificates have time validity)
4. WiFi interference/packet loss during TLS handshake

**Solution:**
1. Check device time is correct (should sync from server)
2. Move closer to WiFi router (reduce interference)
3. Restart device
4. This is OK - HTTP fallback ensures device continues working

**What to do:**
- Monitor for this pattern in serial logs
- If it happens every boot, suggest checking device time
- If happens occasionally, likely WiFi interference - normal

---

### Issue 5: API Response Code 401 (Unauthorized)

**Serial Output:**
```
[Strava] Response code: 401
```

**Root Causes:**
1. API key invalid
2. API key expired
3. Wrong API key format
4. API key not sent correctly

**Solution:**
1. Check API_KEY in config.hpp is correct (64-char hex string)
2. Verify API key hasn't expired on server
3. Verify X-API-Key header is being sent: Look for "Sending request" message
4. Re-generate API key on server if needed

**Debug:**
```bash
# From computer - test API key directly:
curl -H "X-API-Key: YOUR_API_KEY" https://matrix-miles-production.up.railway.app/api/activities/calendar/1/2025/11

# Should return JSON array, not 401
```

---

### Issue 6: API Response Code 404 (Not Found)

**Serial Output:**
```
[Strava] Response code: 404
```

**Root Causes:**
1. Wrong SERVER_BASE_URL
2. Wrong USER_ID
3. Endpoint doesn't exist
4. Path constructed incorrectly

**Solution:**
1. Verify SERVER_BASE_URL in config.hpp: Should be `https://matrix-miles-production.up.railway.app/api`
2. Verify USER_ID = 1 (assuming user 1 exists in database)
3. Check server has the calendar endpoint implemented
4. Look at "Fetching calendar data: /api/activities/calendar/..." in logs

**Debug:**
```bash
# From computer - test the exact endpoint:
curl -H "X-API-Key: YOUR_API_KEY" https://matrix-miles-production.up.railway.app/api/activities/calendar/1/2025/11

# Should return 200 OK with JSON array, not 404
```

---

### Issue 7: Response Code 200 but No Data Parsed

**Serial Output:**
```
[Strava] Response code: 200
[Strava] Response size: 1284
[Strava] Parsed 0 days with activities
```

**Root Causes:**
1. JSON format wrong
2. Response not actually JSON
3. ArduinoJSON parsing issue
4. Empty JSON array (no activities)

**Solution:**
1. Check response format: Look for "Response: [...]" in logs
2. Verify server returns valid JSON array
3. Check for empty array `[]` (valid, just no activities that month)
4. Verify dates in response are in YYYY-MM-DD format

**Debug:**
```bash
# From computer - check response format:
curl -H "X-API-Key: YOUR_API_KEY" https://matrix-miles-production.up.railway.app/api/activities/calendar/1/2025/11

# Expected response format:
[
  {"activity_date":"2025-11-01","count":1,"total_distance":"5.5"},
  {"activity_date":"2025-11-05","count":2,"total_distance":"12.3"}
]

# If empty array [] - that's OK, just no activities that month
```

---

### Issue 8: Device Displays Black Matrix

**Likely Cause:** Calendar data never fetched

**What to check in serial:**
1. Look for "Parsed X days with activities"
2. If not present, data fetch failed
3. Check for errors above

**Solution:**
1. Work through Issues 1-7 above
2. Once data fetches, matrix should display calendar
3. If data fetches but matrix still black, matrix initialization issue (separate)

---

## Enabling Debug Logging

### Verbose Serial Output

Add to esp32_client_cpp.ino in setup():
```cpp
// Enable more debug output
Serial.begin(115200);
Serial.println("\n\n=== DEBUG MODE ===");
Serial.println("If HTTPS fails, will log certificate chain:");
// This happens automatically in connectSSL failure
```

### Check WiFiNINA Debug

Already enabled by default. To increase verbosity:
```cpp
// At top of WiFiManager.cpp, add:
#define WIFIDEBUG 1
#include <WiFiDebug.h>
```

---

## Serial Output Logging

### Capture Full Boot Sequence

```bash
cd esp32_client_cpp

# Connect device and capture output
make monitor PORT=/dev/ttyACM0 > boot_log.txt 2>&1

# Wait for 5 minutes (one API cycle), then Ctrl-C

# Review log
cat boot_log.txt | grep -E "\[Strava\]|\[WiFi\]|\[Test\]"
```

### Save to File (macOS/Linux)

```bash
# Terminal session:
script ~/matrix-debug.log
make monitor PORT=/dev/ttyACM0

# Do something, then Ctrl-D to end session
# Log saved to ~/matrix-debug.log
```

---

## Network Diagnostics

### Check DNS from Device

Add to esp32_client_cpp.ino (temporary):
```cpp
// After WiFi connects:
IPAddress ip = WiFi.getHostByName("matrix-miles-production.up.railway.app");
Serial.print("[DEBUG] DNS lookup result: ");
Serial.println(ip);
```

### Check Certificate Chain

From development computer:
```bash
# Show full certificate chain:
openssl s_client -connect matrix-miles-production.up.railway.app:443 \
  -showcerts -servername matrix-miles-production.up.railway.app 2>/dev/null

# Show certificate details:
echo | openssl s_client -connect matrix-miles-production.up.railway.app:443 \
  -servername matrix-miles-production.up.railway.app 2>/dev/null | \
  openssl x509 -text -noout

# Show certificate expiry:
echo | openssl s_client -connect matrix-miles-production.up.railway.app:443 \
  -servername matrix-miles-production.up.railway.app 2>/dev/null | \
  openssl x509 -noout -dates
```

### Test HTTPS Connection Speed

```bash
# From device (add to code temporarily):
unsigned long start = millis();
WiFiClient client;
bool connected = client.connectSSL("matrix-miles-production.up.railway.app", 443);
unsigned long elapsed = millis() - start;
Serial.print("[PERF] HTTPS connect time: ");
Serial.println(elapsed);
```

---

## Memory Debugging

### Check Available Heap

Add to esp32_client_cpp.ino:
```cpp
// Periodically log free memory:
Serial.print("[MEM] Free heap: ");
Serial.println(freeRam());  // Defined in Arduino core

int freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
```

### String Size Issues

If response is larger than expected:
```cpp
// In StravaClient.cpp, check:
String response = "";
while (client.available()) {
  response += client.readString();
}

// Log size periodically to detect leaks:
Serial.print("[MEM] Response string size: ");
Serial.println(response.length());
```

---

## Certificate Debugging

### Verify Certificate Is Let's Encrypt

```bash
openssl s_client -connect matrix-miles-production.up.railway.app:443 \
  -servername matrix-miles-production.up.railway.app 2>/dev/null | \
  openssl x509 -noout -issuer

# Should show: issuer=C = US, O = Let's Encrypt, CN = R3
```

### Extract Fingerprint Multiple Ways

```bash
# SHA256 fingerprint (what we use):
echo | openssl s_client -connect matrix-miles-production.up.railway.app:443 \
  -servername matrix-miles-production.up.railway.app 2>/dev/null | \
  openssl x509 -noout -fingerprint -sha256 | sed 's/.*=//' | tr -d ':'

# SHA1 fingerprint (alternative):
echo | openssl s_client -connect matrix-miles-production.up.railway.app:443 \
  -servername matrix-miles-production.up.railway.app 2>/dev/null | \
  openssl x509 -noout -fingerprint -sha1 | sed 's/.*=//' | tr -d ':'

# Public key fingerprint:
echo | openssl s_client -connect matrix-miles-production.up.railway.app:443 \
  -servername matrix-miles-production.up.railway.app 2>/dev/null | \
  openssl x509 -noout -pubkey | openssl sha256 -hex
```

---

## Fallback Logic Testing

### Force HTTPS Failure (Testing)

To test HTTP fallback mechanism:

**Method 1: Block port 443 locally**
```bash
# On your WiFi router: Block port 443 for device's IP
# Device should fallback to HTTP
# Check serial: "HTTPS failed, attempting HTTP fallback"
```

**Method 2: Temporary code change**
```cpp
// In StravaClient.cpp fetchCalendarData():
// Temporarily always fail HTTPS:
// if (!useHTTP) {
//   connected = false;  // Force fallback
// } else {
```

**Expected Output:**
```
[Strava] Connection failed after 100ms
[Strava] HTTPS failed, attempting HTTP fallback...
[Strava] ✓ HTTP fallback succeeded in 50ms
```

---

## Performance Monitoring

### Track Connection Times

```cpp
// Add to StravaClient.cpp to monitor trends:
unsigned long httpsTime = 0;
unsigned long httpTime = 0;
unsigned long parseTime = 0;

Serial.print("[PERF] HTTPS: ");
Serial.print(httpsTime);
Serial.print("ms, Parse: ");
Serial.print(parseTime);
Serial.println("ms");
```

### Identify Slow Connections

Look for:
- HTTPS > 500ms (very slow, check WiFi signal)
- Parse > 200ms (check JSON size, might be too large)
- Total fetch > 2000ms (check network congestion)

---

## Getting Help

### Information to Collect

When troubleshooting, provide:
1. Full serial output (from boot to failed operation)
2. Device used: MatrixPortal M4
3. WiFi signal strength: Look for "Signal: -XX dBm"
4. Last working configuration (if any)
5. Changes made before failure

### Minimal Test Case

To diagnose issues, create minimal test:
```cpp
// Minimal HTTPS test in separate sketch:
#include <WiFiNINA.h>

void setup() {
  Serial.begin(115200);
  WiFi.begin("black_mesa", "password");
  
  while (WiFi.status() != WL_CONNECTED) delay(100);
  
  WiFiClient client;
  if (client.connectSSL("matrix-miles-production.up.railway.app", 443)) {
    Serial.println("SUCCESS: HTTPS connection works");
  } else {
    Serial.println("FAILURE: HTTPS connection failed");
  }
}

void loop() {}
```

---

## Summary

When debugging HTTPS issues:

1. **Check WiFi first** - Device must have internet
2. **Check server health** - Test from computer
3. **Check certificate** - Verify it's valid and not expired
4. **Check API key** - Verify it's correct format
5. **Check network** - Firewall, ports 443 and 80 open
6. **Check device time** - Certificates have time validity windows
7. **HTTP fallback OK** - Device still works, just unencrypted

If all else fails, HTTP fallback keeps device functional while you troubleshoot HTTPS.
