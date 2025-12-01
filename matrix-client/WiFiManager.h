#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFiNINA.h>

class WiFiManager {
public:
  WiFiManager();
  
  /// Initialize WiFi with SSID and password
  /// Returns true if connected successfully within timeout
  bool connect(const char* ssid, const char* password, uint32_t timeoutMs = 10000);
  
  /// Check if currently connected to WiFi
  bool isConnected() const;
  
  /// Get the current WiFi SSID
  String getSSID() const;
  
  /// Get the current WiFi IP address
  String getIPAddress() const;
  
  /// Get WiFi signal strength (RSSI)
  int32_t getRSSI() const;
  
  /// Reconnect if disconnected (call in loop)
  void maintain();
  
  /// Test connectivity to a remote host (basic DNS + TCP test)
  bool testConnectivity(const char* testHost, uint16_t testPort = 443);
  
  /// Test HTTP connection (non-SSL) for debugging HTTPS issues
  bool testHTTPConnectivity(const char* testHost);

private:
  bool connected;
};

#endif // WIFI_MANAGER_H
