#ifndef STRAVA_CLIENT_H
#define STRAVA_CLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>

// Calendar entry structure - represents a day with activities
struct CalendarDay {
  int day;           // Day of month (1-31)
  int activityCount; // Number of activities on this day
  float distance;    // Total distance in km
};

class StravaClient {
public:
   StravaClient(const char* apiKey, const char* baseUrl, int userId);
   
   /// Set whether to use HTTP (true) or HTTPS (false) - defaults to HTTP
   /// Set to false if using HTTPS endpoint
   void setUseHTTP(bool useHTTP) { this->useHTTP = useHTTP; }
   
   /// Fetch calendar data for current month from server
   /// Returns number of days with activities found
   int fetchCurrentMonthCalendar(CalendarDay* days, int maxDays);
   
   /// Fetch calendar data for specific month/year
   /// Returns number of days with activities found
   int fetchCalendarData(int year, int month, CalendarDay* days, int maxDays);
   
   /// Test the base URL with a simple GET request (diagnostic)
   /// Logs the full HTTP request/response for debugging
   void testConnection();
   
    /// Synchronize system time from server response headers
    /// Parses HTTP Date header and sets system time
    bool syncTimeFromServer();
    
    /// Get the synchronized date (month, year from server sync)
    void getSyncedDate(int& day, int& month, int& year);
    
    /// Trigger activity sync on the server (calls POST /api/sync/:userId)
    /// This pulls the latest activities from Strava for the current month
    bool syncActivitiesWithServer();

private:
   const char* API_KEY;
   const char* BASE_URL;
   int USER_ID;
   bool useHTTP = true;  // Default to HTTP for local network usage
   
   // Synced date from server (fallback if time() doesn't work)
   int syncedDay = 1;
   int syncedMonth = 1;
   int syncedYear = 1970;
   
   /// Parse calendar JSON response
   int parseCalendarResponse(const String& response, CalendarDay* days, int maxDays);
   
   /// Build API URL for calendar endpoint
   String buildCalendarUrl(int year, int month);
   
   /// Extract hostname from base URL (without port)
   String extractHost(const String& baseUrl);
   
   /// Extract port from base URL, returns default based on useHTTP flag
   uint16_t extractPort(const String& baseUrl);
   
   /// Parse HTTP Date header (RFC 1123 format) and set system time
   /// Example: "Fri, 28 Nov 2025 10:30:45 GMT"
   bool parseHTTPDate(const String& dateHeader);
   
   /// Connect to server with BearSSL certificate validation
   bool connectWithSSL(const String& host, uint16_t port);
   
   /// Validate server certificate fingerprint against expected SHA256
   bool validateCertificateFingerprint(const uint8_t* fingerprint);
};

#endif // STRAVA_CLIENT_H
