#include "StravaClient.h"
#include <curl/curl.h>
#include <cstdio>
#include <cstring>
#include <sstream>

// Callback for libcurl to write response data
static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

StravaClient::StravaClient(const std::string& baseUrl, const std::string& apiKey, int userId)
    : baseUrl_(baseUrl), apiKey_(apiKey), userId_(userId) {
    // Remove trailing slash from base URL if present
    if (!baseUrl_.empty() && baseUrl_.back() == '/') {
        baseUrl_.pop_back();
    }
    
    // Initialize libcurl globally (safe to call multiple times)
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

StravaClient::~StravaClient() {
    curl_global_cleanup();
}

std::string StravaClient::httpGet(const std::string& path) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        lastError_ = "Failed to initialize curl";
        return "";
    }
    
    std::string url = baseUrl_ + path;
    std::string response;
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Set headers
    struct curl_slist* headers = nullptr;
    std::string apiKeyHeader = "X-API-Key: " + apiKey_;
    headers = curl_slist_append(headers, apiKeyHeader.c_str());
    headers = curl_slist_append(headers, "User-Agent: MatrixSimulator/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Set callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    // Timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        lastError_ = std::string("curl error: ") + curl_easy_strerror(res);
        response = "";
    } else {
        // Check HTTP status code
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        if (httpCode != 200) {
            lastError_ = "HTTP error: " + std::to_string(httpCode);
            response = "";
        }
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return response;
}

std::string StravaClient::httpPost(const std::string& path) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        lastError_ = "Failed to initialize curl";
        return "";
    }
    
    std::string url = baseUrl_ + path;
    std::string response;
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Set POST
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L);
    
    // Set headers
    struct curl_slist* headers = nullptr;
    std::string apiKeyHeader = "X-API-Key: " + apiKey_;
    headers = curl_slist_append(headers, apiKeyHeader.c_str());
    headers = curl_slist_append(headers, "User-Agent: MatrixSimulator/1.0");
    headers = curl_slist_append(headers, "Content-Length: 0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Set callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    // Timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        lastError_ = std::string("curl error: ") + curl_easy_strerror(res);
        response = "";
    } else {
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        if (httpCode != 200 && httpCode != 201) {
            lastError_ = "HTTP error: " + std::to_string(httpCode);
            response = "";
        }
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return response;
}

bool StravaClient::testConnection() {
    std::string response = httpGet("/health");
    return !response.empty();
}

bool StravaClient::syncTime() {
    std::string response = httpGet("/health");
    if (response.empty()) {
        return false;
    }
    return parseHealthJson(response);
}

void StravaClient::getSyncedDate(int& day, int& month, int& year) const {
    day = syncedDay_;
    month = syncedMonth_;
    year = syncedYear_;
}

bool StravaClient::parseHealthJson(const std::string& json) {
    // Simple JSON parsing for: {"status":"ok","time":"2025-12-11T10:30:45.123456Z"}
    // Find "time":"
    size_t timePos = json.find("\"time\":\"");
    if (timePos == std::string::npos) {
        lastError_ = "No time field in health response";
        return false;
    }
    
    timePos += 8;  // Skip past "time":"
    size_t timeEnd = json.find("\"", timePos);
    if (timeEnd == std::string::npos) {
        lastError_ = "Malformed time field";
        return false;
    }
    
    std::string timeStr = json.substr(timePos, timeEnd - timePos);
    
    // Parse RFC3339: "2025-12-11T10:30:45.123456Z"
    int year, month, day, hour, minute, second;
    if (sscanf(timeStr.c_str(), "%d-%d-%dT%d:%d:%d",
               &year, &month, &day, &hour, &minute, &second) != 6) {
        lastError_ = "Could not parse time: " + timeStr;
        return false;
    }
    
    syncedYear_ = year;
    syncedMonth_ = month;
    syncedDay_ = day;
    
    return true;
}

std::vector<CalendarDay> StravaClient::fetchCalendarData(int year, int month) {
    std::vector<CalendarDay> result;
    
    std::ostringstream path;
    path << "/api/activities/calendar/" << userId_ << "/" << year << "/" << month;
    
    std::string response = httpGet(path.str());
    if (response.empty()) {
        return result;
    }
    
    return parseCalendarJson(response);
}

std::vector<CalendarDay> StravaClient::parseCalendarJson(const std::string& json) {
    std::vector<CalendarDay> result;
    
    // Simple JSON array parsing for:
    // [{"activity_date":"2025-12-05","count":1,"total_distance":5.2}, ...]
    
    // Find array start
    size_t pos = json.find('[');
    if (pos == std::string::npos) {
        lastError_ = "Response is not a JSON array";
        return result;
    }
    
    // Parse each object in the array
    while (pos < json.length()) {
        // Find next object
        size_t objStart = json.find('{', pos);
        if (objStart == std::string::npos) break;
        
        size_t objEnd = json.find('}', objStart);
        if (objEnd == std::string::npos) break;
        
        std::string obj = json.substr(objStart, objEnd - objStart + 1);
        
        // Parse activity_date
        size_t datePos = obj.find("\"activity_date\":\"");
        if (datePos != std::string::npos) {
            datePos += 17;
            size_t dateEnd = obj.find("\"", datePos);
            if (dateEnd != std::string::npos) {
                std::string dateStr = obj.substr(datePos, dateEnd - datePos);
                
                // Parse YYYY-MM-DD
                int year, month, day;
                if (sscanf(dateStr.c_str(), "%d-%d-%d", &year, &month, &day) == 3) {
                    CalendarDay calDay;
                    calDay.day = day;
                    
                    // Parse count
                    size_t countPos = obj.find("\"count\":");
                    if (countPos != std::string::npos) {
                        countPos += 8;
                        calDay.activityCount = std::stoi(obj.substr(countPos));
                    }
                    
                    // Parse total_distance
                    size_t distPos = obj.find("\"total_distance\":");
                    if (distPos != std::string::npos) {
                        distPos += 17;
                        // Handle both string and number formats
                        if (obj[distPos] == '"') {
                            distPos++;
                            calDay.distance = std::stof(obj.substr(distPos));
                        } else {
                            calDay.distance = std::stof(obj.substr(distPos));
                        }
                    }
                    
                    result.push_back(calDay);
                }
            }
        }
        
        pos = objEnd + 1;
    }
    
    return result;
}

bool StravaClient::syncActivities() {
    std::ostringstream path;
    path << "/api/sync/" << userId_;
    
    std::string response = httpPost(path.str());
    return !response.empty();
}
