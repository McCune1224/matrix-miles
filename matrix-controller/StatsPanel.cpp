#include "StatsPanel.h"
#include <ctime>
#include <cstring>
#include <cstdio>

StatsPanel::StatsPanel()
    : currentDay_(1)
    , month_(1)
    , year_(1970)
    , activityCount_(0)
    , weekActivities_(0)
    , weekDistance_(0.0f)
    , monthActivities_(0)
    , monthDistance_(0.0f) {
    memset(activities_, 0, sizeof(activities_));
}

void StatsPanel::onEnter() {
    computeStats();
}

void StatsPanel::update(uint32_t deltaMs) {
    // No animation needed
}

void StatsPanel::setCurrentDate(int day, int month, int year) {
    currentDay_ = day;
    month_ = month;
    year_ = year;
    computeStats();
}

void StatsPanel::setActivities(const CalendarDay* activities, int count) {
    if (count > 31) count = 31;
    activityCount_ = count;
    for (int i = 0; i < count; i++) {
        activities_[i] = activities[i];
    }
    computeStats();
}

int StatsPanel::getDayOfWeek(int day) const {
    struct tm timeStruct;
    memset(&timeStruct, 0, sizeof(struct tm));
    timeStruct.tm_year = year_ - 1900;
    timeStruct.tm_mon = month_ - 1;
    timeStruct.tm_mday = day;
    timeStruct.tm_isdst = -1;
    mktime(&timeStruct);
    return timeStruct.tm_wday;  // 0=Sunday
}

void StatsPanel::computeStats() {
    // Reset stats
    weekActivities_ = 0;
    weekDistance_ = 0.0f;
    monthActivities_ = 0;
    monthDistance_ = 0.0f;
    
    if (year_ <= 1970) return;
    
    // Find start of current week (Sunday)
    int currentDow = getDayOfWeek(currentDay_);
    int weekStartDay = currentDay_ - currentDow;
    if (weekStartDay < 1) weekStartDay = 1;
    
    // Sum up stats
    for (int i = 0; i < activityCount_; i++) {
        int day = activities_[i].day;
        float dist = activities_[i].distance;
        int count = activities_[i].activityCount;
        
        // Month totals
        monthActivities_ += count;
        monthDistance_ += dist;
        
        // Week totals (Sunday to current day)
        if (day >= weekStartDay && day <= currentDay_) {
            weekActivities_ += count;
            weekDistance_ += dist;
        }
    }
}

void StatsPanel::render(IDisplay* display) {
    display->fillScreen(0);
    
    uint16_t white = display->color565(255, 255, 255);
    uint16_t green = display->color565(0, 255, 0);
    uint16_t gray = display->color565(100, 100, 100);
    
    display->setTextSize(1);
    
    // Convert distances to miles (distance is in meters)
    float weekMiles = weekDistance_ * 0.621371f / 1000.0f;
    float monthMiles = monthDistance_ * 0.621371f / 1000.0f;
    
    // Layout: Two rows, Week on top, Month on bottom
    // Each row: "WK: Xmi Xrun" format
    
    // Row 1: Week stats (Y=4)
    display->setTextColor(gray);
    display->setCursor(0, 4);
    display->print("WK");
    
    display->setTextColor(green);
    display->setCursor(16, 4);
    char weekStr[16];
    snprintf(weekStr, sizeof(weekStr), "%dmi %dr", (int)weekMiles, weekActivities_);
    display->print(weekStr);
    
    // Row 2: Month stats (Y=16)
    display->setTextColor(gray);
    display->setCursor(0, 16);
    display->print("MO");
    
    display->setTextColor(green);
    display->setCursor(16, 16);
    char monthStr[16];
    snprintf(monthStr, sizeof(monthStr), "%dmi %dr", (int)monthMiles, monthActivities_);
    display->print(monthStr);
    
    display->show();
}
