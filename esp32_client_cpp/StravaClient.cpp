#include "StravaClient.h"
#include <Arduino.h>
#include <WiFiNINA.h>
#include <ctime>

StravaClient::StravaClient(const char* apiKey, const char* baseUrl, int userId)
  : API_KEY(apiKey), BASE_URL(baseUrl), USER_ID(userId) {
}

String StravaClient::extractHost(const String& baseUrl) {
  String host = baseUrl;
  host.replace("https://", "");
  host.replace("http://", "");
  
  // Find the path part and remove it
  int slashIndex = host.indexOf('/');
  if (slashIndex > 0) {
    host = host.substring(0, slashIndex);
  }
  
  return host;
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
  
  Serial.print("[Strava] Host: ");
  Serial.println(host);

  // Determine protocol
  Serial.print("[Strava] Using ");
  if (useHTTP) {
    Serial.println("HTTP on port 80");
  } else {
    Serial.println("HTTPS on port 443");
  }
  
  uint16_t port = useHTTP ? 80 : 443;

  // Connect to host
  Serial.print("[Strava] Attempting connection to ");
  Serial.print(host.c_str());
  Serial.print(":");
  Serial.println(port);
  
  unsigned long connectStart = millis();
  
  WiFiClient client;
  if (!client.connect(host.c_str(), port)) {
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

  Serial.print("[Strava] Base URL: ");
  Serial.println(BASE_URL);
  Serial.print("[Strava] Host: ");
  Serial.println(host);

  // Test HTTP first (most compatible)
  Serial.print("[Strava] Attempting HTTP connection to ");
  Serial.print(host.c_str());
  Serial.println(":80");
  
  unsigned long connectStart = millis();
  WiFiClient httpClient;
  if (!httpClient.connect(host.c_str(), 80)) {
    unsigned long connectTime = millis() - connectStart;
    Serial.print("[Strava] ✗ HTTP connection failed after ");
    Serial.print(connectTime);
    Serial.println("ms");
    Serial.println("[Strava] → Recommendation: Check if server supports HTTP or is reachable");
    
    // Try HTTPS
    Serial.print("[Strava] Retrying with HTTPS on port 443...");
    connectStart = millis();
    WiFiClient httpsClient;
    if (httpsClient.connectSSL(host.c_str(), 443)) {
      connectTime = millis() - connectStart;
      Serial.print("\n[Strava] ✓ HTTPS connection succeeded in ");
      Serial.print(connectTime);
      Serial.println("ms");
      httpsClient.stop();
    } else {
      connectTime = millis() - connectStart;
      Serial.print("\n[Strava] ✗ HTTPS also failed after ");
      Serial.print(connectTime);
      Serial.println("ms");
    }
  } else {
    unsigned long connectTime = millis() - connectStart;
    Serial.print("[Strava] ✓ HTTP connection succeeded in ");
    Serial.print(connectTime);
    Serial.println("ms");
    
    // Send health check request
    String request = "GET /health HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: close\r\n";
    request += "\r\n";

    Serial.println("[Strava] === Sending Health Check Request ===");
    Serial.println(request);

    // Send request
    size_t sent = httpClient.print(request);
    Serial.print("[Strava] Bytes sent: ");
    Serial.println(sent);

    // Wait a bit for response
    delay(500);

    // Read and log response data
    Serial.println("[Strava] === HTTP Response ===");
    unsigned long bytesRead = 0;
    unsigned long timeout = millis() + 5000;
    
    while (httpClient.available() && millis() < timeout) {
      String line = httpClient.readStringUntil('\n');
      Serial.println(line);
      bytesRead += line.length() + 1;
    }
    
    Serial.print("[Strava] Total bytes received: ");
    Serial.println(bytesRead);
    
    httpClient.stop();
  }
  
  Serial.println("[Strava] === End Test ===\n");
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
  uint16_t port = useHTTP ? 80 : 443;
  
  WiFiClient client;
  if (!client.connect(host.c_str(), port)) {
    Serial.println("[Strava] Time sync: Connection failed");
    return false;
  }
  
  Serial.println("[Strava] Connected to server for time sync");
  
  // Send simple HEAD request to /health to get date headers
  String request = "HEAD /health HTTP/1.1\r\n";
  request += "Host: " + host + "\r\n";
  request += "Connection: close\r\n";
  request += "\r\n";
  
  client.print(request);
  
  // Read response headers looking for Date
  String dateHeader = "";
  bool foundDate = false;
  
  while (client.available()) {
    String line = client.readStringUntil('\n');
    
    // Remove trailing \r if present
    if (line.length() > 0 && line[line.length()-1] == '\r') {
      line = line.substring(0, line.length()-1);
    }
    
    // Look for Date header
    if (line.startsWith("Date:")) {
      dateHeader = line.substring(6);  // Skip "Date: "
      dateHeader.trim();
      foundDate = true;
      break;
    }
  }
  
  client.stop();
  
  if (!foundDate) {
    Serial.println("[Strava] Time sync: No Date header found");
    return false;
  }
  
  Serial.print("[Strava] Found Date header: ");
  Serial.println(dateHeader);
  
  return parseHTTPDate(dateHeader);
}
