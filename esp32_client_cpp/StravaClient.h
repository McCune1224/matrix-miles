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
  
  /// Set whether to use HTTP (true) or HTTPS (false) - defaults to HTTPS
  /// Set to true if you're having certificate issues
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

private:
  const char* API_KEY;
  const char* BASE_URL;
  int USER_ID;
  bool useHTTP = false;  // Set to true to use HTTP instead of HTTPS
  
  /// Parse calendar JSON response
  int parseCalendarResponse(const String& response, CalendarDay* days, int maxDays);
  
  /// Build API URL for calendar endpoint
  String buildCalendarUrl(int year, int month);
  
  /// Extract hostname from base URL
  String extractHost(const String& baseUrl);
  
  /// Parse HTTP Date header (RFC 1123 format) and set system time
  /// Example: "Fri, 28 Nov 2025 10:30:45 GMT"
  bool parseHTTPDate(const String& dateHeader);
};

#endif // STRAVA_CLIENT_H
