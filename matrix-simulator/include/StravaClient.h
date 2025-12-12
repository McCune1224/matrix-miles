#ifndef STRAVA_CLIENT_H
#define STRAVA_CLIENT_H

#include <string>
#include <vector>
#include "MockData.h"  // Reuse CalendarDay struct

class StravaClient {
public:
    StravaClient(const std::string& baseUrl, const std::string& apiKey, int userId);
    ~StravaClient();
    
    // Fetch calendar data for a specific month
    // Returns vector of CalendarDay with activity data
    std::vector<CalendarDay> fetchCalendarData(int year, int month);
    
    // Sync time from server's /health endpoint
    // Returns true on success, populates day/month/year
    bool syncTime();
    
    // Get the last synced date
    void getSyncedDate(int& day, int& month, int& year) const;
    
    // Trigger activity sync on server (POST /api/sync/:userId)
    bool syncActivities();
    
    // Test connection to server
    bool testConnection();
    
    // Get last error message
    const std::string& getLastError() const { return lastError_; }
    
private:
    std::string baseUrl_;
    std::string apiKey_;
    int userId_;
    
    // Synced date from server
    int syncedDay_ = 1;
    int syncedMonth_ = 1;
    int syncedYear_ = 1970;
    
    std::string lastError_;
    
    // HTTP request helpers
    std::string httpGet(const std::string& path);
    std::string httpPost(const std::string& path);
    
    // JSON parsing helpers
    std::vector<CalendarDay> parseCalendarJson(const std::string& json);
    bool parseHealthJson(const std::string& json);
};

#endif // STRAVA_CLIENT_H
