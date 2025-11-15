# ESP32 Production Configuration Guide

## Quick Setup

After deploying your strava-server to Railway, update your ESP32 code with these values:

### 1. Update blink.ino

```cpp
// Configuration - API Key
#define ESP32_API_KEY "your_railway_api_key_here"

// Configuration - Server URLs
#define USE_PRODUCTION true

#if USE_PRODUCTION
  const char* SERVER_URL = "https://your-app-name.up.railway.app";
#else
  const char* SERVER_URL = "http://localhost:8080";
#endif
```

### 2. Get Your Configuration Values

#### Production Server URL
1. Deploy to Railway following `RAILWAY_DEPLOYMENT.md`
2. Your URL will be: `https://your-app-name.up.railway.app`
3. You can also set a custom domain in Railway settings

#### API Key
Generate a secure API key:
```bash
openssl rand -hex 32
```

Use this SAME key in:
- Railway environment variables (`ESP32_API_KEY`)
- ESP32 code (`#define ESP32_API_KEY`)

#### User ID
1. Visit: `https://your-app-name.up.railway.app/auth/login`
2. Authorize with Strava
3. Note your User ID (e.g., `1`, `2`, `3`)
4. Use in API requests: `/api/activities/recent/YOUR_USER_ID`

### 3. ESP32 API Endpoints

```cpp
// Get recent activities
http.begin(SERVER_URL "/api/activities/recent/1");
http.addHeader("X-API-Key", ESP32_API_KEY);
int httpCode = http.GET();

// Get calendar data (year/month)
http.begin(SERVER_URL "/api/activities/calendar/1/2025/11");
http.addHeader("X-API-Key", ESP32_API_KEY);
int httpCode = http.GET();

// Get user stats
http.begin(SERVER_URL "/api/stats/1");
http.addHeader("X-API-Key", ESP32_API_KEY);
int httpCode = http.GET();
```

### 4. Complete ESP32 Example

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#define USE_SERIAL Serial

// ===== CONFIGURATION =====
// API Key (generate with: openssl rand -hex 32)
#define ESP32_API_KEY "9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6"

// Server URLs
#define USE_PRODUCTION true

#if USE_PRODUCTION
  const char* SERVER_URL = "https://your-app-name.up.railway.app";
#else
  const char* SERVER_URL = "http://localhost:8080";
#endif

// Your user ID (from OAuth login)
const int USER_ID = 1;
// ========================

WiFiMulti wifiMulti;

void setup() {
  USE_SERIAL.begin(115200);
  USE_SERIAL.println("\n\n\nStrava ESP32 Client Starting...");

  // Connect to WiFi
  wifiMulti.addAP("your_ssid", "your_password");
  
  USE_SERIAL.print("Connecting to WiFi");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    USE_SERIAL.print(".");
  }
  USE_SERIAL.println(" Connected!");
}

void loop() {
  if (wifiMulti.run() == WL_CONNECTED) {
    HTTPClient http;
    
    // Build URL
    String url = String(SERVER_URL) + "/api/activities/recent/" + String(USER_ID);
    
    USE_SERIAL.printf("[HTTP] Requesting: %s\n", url.c_str());
    
    // Begin connection
    http.begin(url);
    
    // Add API key header
    http.addHeader("X-API-Key", ESP32_API_KEY);
    
    // Send GET request
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      USE_SERIAL.printf("[HTTP] Response code: %d\n", httpCode);
      
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        USE_SERIAL.println("[HTTP] Response:");
        USE_SERIAL.println(payload);
      }
    } else {
      USE_SERIAL.printf("[HTTP] Request failed: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
  }
  
  // Wait 30 seconds before next request
  delay(30000);
}
```

## Testing

### Test Production Server Health

```bash
curl https://your-app-name.up.railway.app/health
```

Expected response:
```json
{
  "status": "ok",
  "time": "2025-11-14T12:00:00Z"
}
```

### Test API Key Authentication

```bash
curl -H "X-API-Key: your_api_key" \
  https://your-app-name.up.railway.app/api/activities/recent/1
```

### Common Issues

**401 Unauthorized**
- Check ESP32_API_KEY matches Railway environment variable
- Verify header name is `X-API-Key` (case-sensitive)

**404 Not Found**
- Verify User ID exists (complete OAuth flow first)
- Check URL format: `/api/activities/recent/{userId}`

**Connection Failed**
- Check WiFi connection on ESP32
- Verify Railway app is running (check dashboard)
- Test with curl first before ESP32

## Deployment Checklist

- [ ] Deploy strava-server to Railway
- [ ] Add PostgreSQL database
- [ ] Set all environment variables in Railway
- [ ] Run database migrations
- [ ] Update Strava API callback URL
- [ ] Test OAuth flow and get User ID
- [ ] Generate and set API key
- [ ] Update ESP32 code with production URL and API key
- [ ] Test API endpoints with curl
- [ ] Flash ESP32 with production code
- [ ] Monitor Railway logs for requests

## Security Reminders

- ✅ Never commit API keys to git
- ✅ Use different API keys for test/production
- ✅ Rotate API keys periodically
- ✅ Monitor Railway logs for suspicious activity
- ✅ Use HTTPS for all production requests (Railway default)

## Support

- Railway Dashboard: https://railway.app/dashboard
- Railway Logs: Check "View Logs" in deployment
- Strava API Status: https://status.strava.com
- ESP32 Serial Monitor: Monitor output at 115200 baud
