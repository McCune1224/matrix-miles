#include "WiFiManager.h"

WiFiManager::WiFiManager() : connected(false) {
}

bool WiFiManager::connect(const char* ssid, const char* password, uint32_t timeoutMs) {
  if (!ssid || !password) {
    Serial.println("[WiFi] Error: SSID and password cannot be null");
    return false;
  }

  Serial.print("[WiFi] Connecting to SSID: ");
  Serial.println(ssid);
  
  // Connection loop with timeout
  uint32_t startTime = millis();
  int attempts = 0;
  
  while (millis() - startTime < timeoutMs) {
    // Check WiFi status
    int status = WiFi.status();
    
    if (status == WL_CONNECTED) {
      connected = true;
      Serial.print("[WiFi] Connected to: ");
      Serial.println(WiFi.SSID());
      Serial.print("[WiFi] IP Address: ");
      Serial.println(WiFi.localIP());
      Serial.print("[WiFi] Signal Strength (RSSI): ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");
      return true;
    }
    
    // Only attempt to connect if not already connected or connecting
    if (status != WL_CONNECTED) {
      WiFi.begin(ssid, password);
    }
    
    attempts++;
    if (attempts % 20 == 0) {
      Serial.print(".");
    }
    
    delay(500);
  }
  
  connected = false;
  Serial.println();
  Serial.println("[WiFi] Connection timeout - failed to connect");
  return false;
}

bool WiFiManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getSSID() const {
  if (isConnected()) {
    return WiFi.SSID();
  }
  return "Not Connected";
}

String WiFiManager::getIPAddress() const {
  if (isConnected()) {
    IPAddress ip = WiFi.localIP();
    char buf[16];
    sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return String(buf);
  }
  return "N/A";
}

int32_t WiFiManager::getRSSI() const {
  if (isConnected()) {
    return WiFi.RSSI();
  }
  return 0;
}

void WiFiManager::maintain() {
  // Monitor connection status
  int status = WiFi.status();
  
  if (status != WL_CONNECTED && connected) {
    connected = false;
    Serial.println("[WiFi] Connection lost, attempting to reconnect...");
  }
}

bool WiFiManager::testConnectivity(const char* testHost, uint16_t testPort) {
  if (!isConnected()) {
    Serial.println("[WiFi] Not connected to WiFi");
    return false;
  }
  
  Serial.print("[WiFi] Testing connectivity to ");
  Serial.print(testHost);
  Serial.print(":");
  Serial.println(testPort);
  
  // Try to connect to the host (WiFiNINA resolves during connect)
  WiFiSSLClient testClient;
  Serial.print("[WiFi] Attempting TCP connection...");
  
  unsigned long connectStart = millis();
  if (!testClient.connect(testHost, testPort)) {
    unsigned long connectTime = millis() - connectStart;
    Serial.print(" FAILED after ");
    Serial.print(connectTime);
    Serial.println("ms");
    return false;
  }

  unsigned long connectTime = millis() - connectStart;
  Serial.print(" SUCCESS in ");
  Serial.print(connectTime);
  Serial.println("ms");
  
  testClient.stop();
  return true;
}

bool WiFiManager::testHTTPConnectivity(const char* testHost) {
  if (!isConnected()) {
    Serial.println("[WiFi] Not connected to WiFi");
    return false;
  }
  
  Serial.print("[WiFi] Testing HTTP connectivity to ");
  Serial.print(testHost);
  Serial.println(":80");
  
  // Use regular WiFiClient (non-SSL) to test HTTP
  WiFiClient testClient;
  Serial.print("[WiFi] Attempting HTTP connection...");
  
  unsigned long connectStart = millis();
  if (!testClient.connect(testHost, 80)) {
    unsigned long connectTime = millis() - connectStart;
    Serial.print(" FAILED after ");
    Serial.print(connectTime);
    Serial.println("ms");
    return false;
  }

  unsigned long connectTime = millis() - connectStart;
  Serial.print(" SUCCESS in ");
  Serial.print(connectTime);
  Serial.println("ms");
  
  testClient.stop();
  return true;
}
