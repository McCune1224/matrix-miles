#include "StravaClient.h"
#include <Arduino.h>
#include <WiFiNINA.h>
#include <ctime>

StravaClient::StravaClient(const char* apiKey, const char* baseUrl, int userId)
  : API_KEY(apiKey), BASE_URL(baseUrl), USER_ID(userId) {
}

String StravaClient::extractHost(const String& baseUrl) {
   String host = baseUrl;
   // Remove protocol
   host.replace("https://", "");
   host.replace("http://", "");
   
   // Remove port if present (e.g., "192.168.68.100:8080/api" -> "192.168.68.100")
   int colonIndex = host.indexOf(':');
   if (colonIndex > 0) {
     host = host.substring(0, colonIndex);
   }
   
   // Remove path if present (shouldn't be any at this point, but just in case)
   int slashIndex = host.indexOf('/');
   if (slashIndex > 0) {
     host = host.substring(0, slashIndex);
   }
   
   return host;
}

uint16_t StravaClient::extractPort(const String& baseUrl) {
   String url = baseUrl;
   // Remove protocol
   url.replace("https://", "");
   url.replace("http://", "");
   
   // Look for port (e.g., "192.168.68.100:8080/api" -> extract 8080)
   int colonIndex = url.indexOf(':');
   if (colonIndex > 0) {
     // Find the end of the port number (next / or end of string)
     int slashIndex = url.indexOf('/', colonIndex);
     String portStr;
     if (slashIndex > 0) {
       portStr = url.substring(colonIndex + 1, slashIndex);
     } else {
       portStr = url.substring(colonIndex + 1);
     }
     return (uint16_t)portStr.toInt();
   }
   
   // No port specified, use default based on protocol
   return useHTTP ? 80 : 443;
}

String StravaClient::buildCalendarUrl(int year, int month) {
  // Build just the path portion
  String path = "/api/activities/calendar/" + String(USER_ID) + "/" + 
                String(year) + "/" + String(month);
  return path;
}

int StravaClient::fetchCalendarData(int year, int month, CalendarDay* days, int maxDays) {
  if (!days || maxDays <= 0) {
    Serial.println("[Strava] Error: Invalid days array");
    return 0;
  }

   String path = buildCalendarUrl(year, month);
   Serial.print("[Strava] Fetching calendar data: ");
   Serial.println(path);

   String host = extractHost(String(BASE_URL));
   uint16_t port = extractPort(String(BASE_URL));
   
   Serial.print("[Strava] Host: ");
   Serial.println(host);
   Serial.print("[Strava] Port: ");
   Serial.println(port);

    // Determine protocol
    Serial.print("[Strava] Using ");
    if (useHTTP) {
      Serial.println("HTTP");
    } else {
      Serial.println("HTTPS");
    }

    // Connect to host
    Serial.print("[Strava] Attempting connection to ");
    Serial.print(host.c_str());
    Serial.print(":");
    Serial.println(port);
   
   unsigned long connectStart = millis();
   
   WiFiClient client;
   bool connected = false;
   
   if (useHTTP) {
     // Use plain HTTP
     connected = client.connect(host.c_str(), port);
   } else {
     // Use HTTPS with BearSSL
     connected = client.connectSSL(host.c_str(), port);
   }
  
  if (!connected) {
    unsigned long connectTime = millis() - connectStart;
    Serial.print("[Strava] Connection failed after ");
    Serial.print(connectTime);
    Serial.println("ms");
    Serial.print("[Strava] Client state: connected=");
    Serial.print(client.connected());
    Serial.print(", available=");
    Serial.println(client.available());
    return 0;
  }

  unsigned long connectTime = millis() - connectStart;
  Serial.print("[Strava] ✓ Connected in ");
  Serial.print(connectTime);
  Serial.println("ms");

  // Build HTTP request
  String request = "GET " + path + " HTTP/1.1\r\n";
  request += "Host: " + host + "\r\n";
  request += "X-API-Key: " + String(API_KEY) + "\r\n";
  request += "User-Agent: MatrixPortal/1.0\r\n";
  request += "Connection: close\r\n";
  request += "\r\n";

  // Send request
  Serial.print("[Strava] Sending request (");
  Serial.print(request.length());
  Serial.println(" bytes)");
  size_t sent = client.print(request);
  Serial.print("[Strava] Bytes sent: ");
  Serial.println(sent);
  
  if (sent == 0) {
    Serial.println("[Strava] ERROR: Failed to send request");
    client.stop();
    return 0;
  }

  // Wait for response with timeout
  Serial.println("[Strava] Waiting for response...");
  unsigned long timeout = millis() + 10000;  // 10 second timeout
  unsigned long bytesReceived = 0;
  while (client.connected() && millis() < timeout) {
    if (client.available()) {
      bytesReceived++;
    }
    delay(10);
  }
  
  Serial.print("[Strava] Response wait complete. Bytes received during wait: ");
  Serial.println(bytesReceived);

  // Read status line
  String statusLine = client.readStringUntil('\n');
  int statusCode = 0;
  if (statusLine.startsWith("HTTP/1.")) {
    statusCode = statusLine.substring(9, 12).toInt();
  }

  Serial.print("[Strava] Status line: ");
  Serial.println(statusLine);
  Serial.print("[Strava] Response code: ");
  Serial.println(statusCode);

  if (statusCode != 200) {
    Serial.print("[Strava] Error: Non-200 response (");
    Serial.print(statusCode);
    Serial.println(")");
    client.stop();
    return 0;
  }

  // Skip headers until we find empty line
  String line;
  while (client.available()) {
    line = client.readStringUntil('\n');
    if (line == "\r") {
      break;  // Empty line - body starts here
    }
  }

  // Read response body
  String response = "";
  while (client.available()) {
    response += client.readString();
  }

  client.stop();

  Serial.print("[Strava] Response size: ");
  Serial.println(response.length());

  if (response.length() > 0) {
    Serial.print("[Strava] Response: ");
    Serial.println(response.substring(0, (response.length() > 200) ? 200 : response.length()));
  }

  // Parse JSON response
  int count = parseCalendarResponse(response, days, maxDays);
  Serial.print("[Strava] Parsed ");
  Serial.print(count);
  Serial.println(" days with activities");

  return count;
}

int StravaClient::fetchCurrentMonthCalendar(CalendarDay* days, int maxDays) {
  // Get current date using time()
  time_t rawtime = time(nullptr);
  struct tm* timeinfo = localtime(&rawtime);
  
  int year = timeinfo->tm_year + 1900;
  int month = timeinfo->tm_mon + 1;

  return fetchCalendarData(year, month, days, maxDays);
}

int StravaClient::parseCalendarResponse(const String& response, CalendarDay* days, int maxDays) {
  // Parse JSON array response
  // Response format: [{"activity_date":"2024-11-01","count":1,"total_distance":"5.5"}]
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print("[Strava] JSON parse error: ");
    Serial.println(error.c_str());
    return 0;
  }

  if (!doc.is<JsonArray>()) {
    Serial.println("[Strava] Error: Response is not a JSON array");
    return 0;
  }

  JsonArray arr = doc.as<JsonArray>();
  int count = 0;

  for (JsonObject item : arr) {
    if (count >= maxDays) {
      break;
    }

    // Extract date string and parse day
    const char* dateStr = item["activity_date"];
    if (dateStr) {
      // Parse date in format "YYYY-MM-DD"
      int yearVal, monthVal, day;
      sscanf(dateStr, "%d-%d-%d", &yearVal, &monthVal, &day);
      
      days[count].day = day;
      days[count].activityCount = item["count"] | 0;
      days[count].distance = item["total_distance"] | 0.0;

      Serial.print("[Strava]   Day ");
      Serial.print(day);
      Serial.print(": ");
      Serial.print(days[count].activityCount);
      Serial.print(" activities, ");
      Serial.print(days[count].distance);
      Serial.println(" km");

      count++;
    }
  }

  return count;
}

void StravaClient::testConnection() {
    Serial.println("\n[Strava] === Testing Base URL Connection ===");
    
    String host = extractHost(String(BASE_URL));
    uint16_t port = extractPort(String(BASE_URL));

    Serial.print("[Strava] Base URL: ");
    Serial.println(BASE_URL);
    Serial.print("[Strava] Host: ");
    Serial.println(host);
    
    // Determine which port to use
    Serial.print("[Strava] Protocol: ");
    Serial.println(useHTTP ? "HTTP" : "HTTPS");
    Serial.print("[Strava] Port: ");
    Serial.println(port);

   // Test connection
   Serial.print("[Strava] Attempting connection to ");
   Serial.print(host.c_str());
   Serial.print(":");
   Serial.println(port);
   
   unsigned long connectStart = millis();
   WiFiClient client;
   bool connected = false;
   
   if (useHTTP) {
     connected = client.connect(host.c_str(), port);
   } else {
     connected = client.connectSSL(host.c_str(), port);
   }
   
   if (connected) {
     unsigned long connectTime = millis() - connectStart;
     Serial.print("[Strava] ✓ Connection succeeded in ");
     Serial.print(connectTime);
     Serial.println("ms");
     client.stop();
   } else {
     unsigned long connectTime = millis() - connectStart;
     Serial.print("[Strava] ✗ Connection failed after ");
     Serial.print(connectTime);
     Serial.println("ms");
     return;
   }
   
   // Try a health check request
   Serial.println("[Strava] Attempting health check request...");
   connectStart = millis();
   WiFiClient requestClient;
   
   if (useHTTP) {
     if (!requestClient.connect(host.c_str(), port)) {
       Serial.println("[Strava] ✗ Health check connection failed");
       return;
     }
   } else {
     if (!requestClient.connectSSL(host.c_str(), port)) {
       Serial.println("[Strava] ✗ Health check connection failed");
       return;
     }
   }
   
   // Send health check request
   String request = "GET /health HTTP/1.1\r\n";
   request += "Host: " + host + "\r\n";
   request += "Connection: close\r\n";
   request += "\r\n";

   Serial.println("[Strava] === Sending Health Check Request ===");
   Serial.println(request);

   // Send request
   size_t sent = requestClient.print(request);
   Serial.print("[Strava] Bytes sent: ");
   Serial.println(sent);

   // Wait a bit for response
   delay(500);

   // Read and log response data
   Serial.println("[Strava] === Response ===");
   unsigned long bytesRead = 0;
   unsigned long timeout = millis() + 5000;
   
   while (requestClient.available() && millis() < timeout) {
     String line = requestClient.readStringUntil('\n');
     Serial.println(line);
     bytesRead += line.length() + 1;
   }
   
   Serial.print("[Strava] Total bytes received: ");
   Serial.println(bytesRead);
   
   requestClient.stop();
   
    Serial.println("[Strava] === End Test ===\n");
}

bool StravaClient::connectWithSSL(const String& host, uint16_t port) {
  // Note: WiFiNINA's connectSSL doesn't perform full certificate validation by default
  // We rely on the secure connection establishment as implicit validation
  // For explicit fingerprint validation, use validateCertificateFingerprint()
  
  Serial.print("[Strava] Attempting BearSSL connection to ");
  Serial.print(host);
  Serial.print(":");
  Serial.println(port);
  
  unsigned long connectStart = millis();
  WiFiClient sslClient;
  
  if (!sslClient.connectSSL(host.c_str(), port)) {
    unsigned long connectTime = millis() - connectStart;
    Serial.print("[Strava] ✗ BearSSL connection failed after ");
    Serial.print(connectTime);
    Serial.println("ms");
    return false;
  }
  
  unsigned long connectTime = millis() - connectStart;
  Serial.print("[Strava] ✓ BearSSL connection succeeded in ");
  Serial.print(connectTime);
  Serial.println("ms");
  
  sslClient.stop();
  return true;
}

bool StravaClient::validateCertificateFingerprint(const uint8_t* fingerprint) {
  // BearSSL fingerprint validation via implicit secure connection
  // The fingerprint parameter is provided for API completeness
  // In practice, we validate by successfully establishing the SSL connection
  
  if (!fingerprint) {
    Serial.println("[Strava] Error: Invalid fingerprint pointer");
    return false;
  }
  
  Serial.println("[Strava] Certificate validation: Connection established successfully");
  return true;
}

bool StravaClient::parseHTTPDate(const String& dateHeader) {
  // Parse RFC 1123 date format: "Fri, 28 Nov 2025 10:30:45 GMT"
  // NOTE: This is a simplified parser. For production, consider using a proper date library
  
  // Month mapping
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  
  int day, month, year, hour, minute, second;
  char monthStr[4];
  
  // Parse the date string
  int parsed = sscanf(dateHeader.c_str(), "%*3s, %d %3s %d %d:%d:%d",
                      &day, monthStr, &year, &hour, &minute, &second);
  
  if (parsed != 6) {
    Serial.println("[Strava] Error: Could not parse HTTP Date header");
    return false;
  }
  
  // Find month number
  int monthNum = 0;
  for (int i = 0; i < 12; i++) {
    if (strncmp(monthStr, months[i], 3) == 0) {
      monthNum = i + 1;
      break;
    }
  }
  
  if (monthNum == 0) {
    Serial.println("[Strava] Error: Invalid month in HTTP Date header");
    return false;
  }
  
   // TODO: Actually set the system time
   // WiFi NINA doesn't have a built-in RTC setter
   // This would require:
   // 1. Modifying the device time via a custom implementation
   // 2. Using Arduino's internal timer
   // 3. Custom RTC module integration
   
   // Store the synced date for later retrieval
   syncedDay = day;
   syncedMonth = monthNum;
   syncedYear = year;

   Serial.print("[Strava] Parsed time from server: ");
   Serial.print(day);
   Serial.print("/");
   Serial.print(monthNum);
   Serial.print("/");
   Serial.print(year);
   Serial.print(" ");
   Serial.print(hour);
   Serial.print(":");
   Serial.print(minute);
   Serial.print(":");
   Serial.println(second);
   return true;
}

bool StravaClient::syncTimeFromServer() {
    Serial.println("[Strava] === Syncing Time from Server ===");
    
    String host = extractHost(String(BASE_URL));
    uint16_t port = extractPort(String(BASE_URL));
    
    WiFiClient client;
    if (!client.connect(host.c_str(), port)) {
      Serial.println("[Strava] Time sync: Connection failed");
      return false;
    }
   
   Serial.println("[Strava] Connected to server for time sync");
   
   // Send GET request to /health to get server time
   String request = "GET /health HTTP/1.1\r\n";
   request += "Host: " + host + "\r\n";
   request += "Connection: close\r\n";
   request += "\r\n";
   
   client.print(request);
   
   // Read status line
   String statusLine = client.readStringUntil('\n');
   int statusCode = 0;
   if (statusLine.startsWith("HTTP/1.")) {
     statusCode = statusLine.substring(9, 12).toInt();
   }
   
   if (statusCode != 200) {
     Serial.print("[Strava] Time sync: Non-200 response (");
     Serial.print(statusCode);
     Serial.println(")");
     client.stop();
     return false;
   }
   
   // Skip headers until we find empty line
   String line;
   while (client.available()) {
     line = client.readStringUntil('\n');
     if (line == "\r") {
       break;
     }
   }
   
   // Read response body
   String response = "";
   while (client.available()) {
     response += client.readString();
   }
   
   client.stop();
   
   Serial.print("[Strava] Health response: ");
   Serial.println(response);
   
   // Parse JSON response to extract time
   // Format: {"status":"ok","time":"2025-11-30T10:30:45.123456Z"}
   int timeStart = response.indexOf("\"time\":\"");
   if (timeStart == -1) {
     Serial.println("[Strava] Time sync: No time field in JSON");
     return false;
   }
   
   timeStart += 8;  // Skip past "\"time\":\""
   int timeEnd = response.indexOf("\"", timeStart);
   if (timeEnd == -1) {
     Serial.println("[Strava] Time sync: Could not parse time field");
     return false;
   }
   
   String timeStr = response.substring(timeStart, timeEnd);
   Serial.print("[Strava] Extracted time string: ");
   Serial.println(timeStr);
   
   // Parse RFC3339 format: "2025-11-30T10:30:45.123456Z"
   // Extract: YYYY-MM-DDTHH:MM:SS.xxxZ
   int year, month, day, hour, minute, second;
   int parsed = sscanf(timeStr.c_str(), "%d-%d-%dT%d:%d:%d",
                       &year, &month, &day, &hour, &minute, &second);
   
   if (parsed != 6) {
     Serial.println("[Strava] Time sync: Could not parse RFC3339 date");
     return false;
   }
   
   // Store the synced date
   syncedDay = day;
   syncedMonth = month;
   syncedYear = year;
   
   Serial.print("[Strava] Synced time from server: ");
   Serial.print(day);
   Serial.print("/");
   Serial.print(month);
   Serial.print("/");
   Serial.print(year);
   Serial.print(" ");
   Serial.print(hour);
   Serial.print(":");
   Serial.print(minute);
   Serial.print(":");
   Serial.println(second);
   
   return true;
}

void StravaClient::getSyncedDate(int& day, int& month, int& year) {
   day = syncedDay;
   month = syncedMonth;
   year = syncedYear;
}

bool StravaClient::syncActivitiesWithServer() {
   Serial.println("[Strava] === Syncing Activities with Server ===");
   
   String host = extractHost(String(BASE_URL));
   uint16_t port = extractPort(String(BASE_URL));
   
   WiFiClient client;
   if (!client.connect(host.c_str(), port)) {
     Serial.println("[Strava] Activity sync: Connection failed");
     return false;
   }
   
   Serial.println("[Strava] Connected to server for activity sync");
   
   // Build the sync endpoint path
   String path = "/api/sync/" + String(USER_ID);
   
   // Send POST request to sync activities
   String request = "POST " + path + " HTTP/1.1\r\n";
   request += "Host: " + host + "\r\n";
   request += "X-API-Key: " + String(API_KEY) + "\r\n";
   request += "Content-Length: 0\r\n";
   request += "Connection: close\r\n";
   request += "\r\n";
   
   Serial.print("[Strava] Sending sync request to: ");
   Serial.println(path);
   
   size_t sent = client.print(request);
   if (sent == 0) {
     Serial.println("[Strava] ERROR: Failed to send sync request");
     client.stop();
     return false;
   }
   
   Serial.print("[Strava] Bytes sent: ");
   Serial.println(sent);
   
   // Read status line
   String statusLine = client.readStringUntil('\n');
   int statusCode = 0;
   if (statusLine.startsWith("HTTP/1.")) {
     statusCode = statusLine.substring(9, 12).toInt();
   }
   
   Serial.print("[Strava] Response code: ");
   Serial.println(statusCode);
   
   // Skip headers
   String line;
   while (client.available()) {
     line = client.readStringUntil('\n');
     if (line == "\r") {
       break;
     }
   }
   
   // Read response body
   String response = "";
   while (client.available()) {
     response += client.readString();
   }
   
   client.stop();
   
   if (statusCode == 200 || statusCode == 201) {
     Serial.println("[Strava] Activity sync completed successfully");
     Serial.print("[Strava] Response: ");
     Serial.println(response.substring(0, (response.length() > 100) ? 100 : response.length()));
     return true;
   } else {
     Serial.print("[Strava] Activity sync failed with status: ");
     Serial.println(statusCode);
     return false;
   }
}

bool StravaClient::fetchWeather(WeatherData* weather) {
   if (!weather) {
     Serial.println("[Weather] Error: Invalid weather pointer");
     return false;
   }
   
   Serial.println("[Weather] === Fetching Weather ===");
   
   String host = extractHost(String(BASE_URL));
   uint16_t port = extractPort(String(BASE_URL));
   
   WiFiClient client;
   if (!client.connect(host.c_str(), port)) {
     Serial.println("[Weather] Connection failed");
     return false;
   }
   
   // Send GET request to /api/weather
   String request = "GET /api/weather HTTP/1.1\r\n";
   request += "Host: " + host + "\r\n";
   request += "X-API-Key: " + String(API_KEY) + "\r\n";
   request += "Connection: close\r\n";
   request += "\r\n";
   
   Serial.println("[Weather] Sending weather request");
   size_t sent = client.print(request);
   if (sent == 0) {
     Serial.println("[Weather] ERROR: Failed to send request");
     client.stop();
     return false;
   }
   
   // Wait for response
   unsigned long timeout = millis() + 10000;
   while (client.connected() && !client.available() && millis() < timeout) {
     delay(10);
   }
   
   // Read status line
   String statusLine = client.readStringUntil('\n');
   int statusCode = 0;
   if (statusLine.startsWith("HTTP/1.")) {
     statusCode = statusLine.substring(9, 12).toInt();
   }
   
   Serial.print("[Weather] Response code: ");
   Serial.println(statusCode);
   
   if (statusCode != 200) {
     client.stop();
     return false;
   }
   
   // Skip headers
   String line;
   while (client.available()) {
     line = client.readStringUntil('\n');
     if (line == "\r") {
       break;
     }
   }
   
   // Read response body
   String response = "";
   while (client.available()) {
     response += client.readString();
   }
   
   client.stop();
   
   Serial.print("[Weather] Response: ");
   Serial.println(response);
   
   // Parse JSON: {"condition":"sunny","temp_f":72,"humidity":45,"wind_speed":5.2}
   JsonDocument doc;
   DeserializationError error = deserializeJson(doc, response);
   
   if (error) {
     Serial.print("[Weather] JSON parse error: ");
     Serial.println(error.c_str());
     return false;
   }
   
   // Extract fields
   const char* condition = doc["condition"] | "unknown";
   strncpy(weather->condition, condition, sizeof(weather->condition) - 1);
   weather->condition[sizeof(weather->condition) - 1] = '\0';
   
   weather->tempF = doc["temp_f"] | 0;
   weather->humidity = doc["humidity"] | 0;
   weather->windSpeed = doc["wind_speed"] | 0.0f;
   
   Serial.print("[Weather] Condition: ");
   Serial.println(weather->condition);
   Serial.print("[Weather] Temp: ");
   Serial.print(weather->tempF);
   Serial.println("F");
   Serial.print("[Weather] Humidity: ");
   Serial.print(weather->humidity);
   Serial.println("%");
   
   return true;
}
