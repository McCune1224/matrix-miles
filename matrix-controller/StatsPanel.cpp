#include "StatsPanel.h"
#include <ctime>
#include <cstring>
#include <cstdio>

StatsPanel::StatsPanel()
    : currentDay_(1)
    , month_(1)
    , year_(1970)
    , activityCount_(0)
    , animTime_(0)
    , totalMiles_(0.0f)
    , goalMiles_(20.0f)  // Hardcoded goal: 20 miles
    , totalRuns_(0)
    , todayDayOfWeek_(0) {
    memset(activities_, 0, sizeof(activities_));
    memset(weeklyMiles_, 0, sizeof(weeklyMiles_));
}

void StatsPanel::onEnter() {
    animTime_ = 0;
    computeStats();
}

void StatsPanel::update(uint32_t deltaMs) {
    animTime_ += deltaMs;
}

void StatsPanel::setCurrentDate(int day, int month, int year) {
    currentDay_ = day;
    month_ = month;
    year_ = year;
    todayDayOfWeek_ = getDayOfWeek(day);
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
    // Reset
    memset(weeklyMiles_, 0, sizeof(weeklyMiles_));
    totalMiles_ = 0.0f;
    totalRuns_ = 0;
    
    if (year_ <= 1970) return;
    
    // Find start of current week (Sunday)
    int currentDow = getDayOfWeek(currentDay_);
    int weekStartDay = currentDay_ - currentDow;
    if (weekStartDay < 1) weekStartDay = 1;
    int weekEndDay = weekStartDay + 6;
    
    // Get days in month for bounds checking
    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month_ == 2 && ((year_ % 4 == 0 && year_ % 100 != 0) || (year_ % 400 == 0))) {
        daysInMonth[2] = 29;
    }
    if (weekEndDay > daysInMonth[month_]) {
        weekEndDay = daysInMonth[month_];
    }
    
    // Sum up stats from activities
    for (int i = 0; i < activityCount_; i++) {
        int day = activities_[i].day;
        float distMeters = activities_[i].distance;
        int count = activities_[i].activityCount;
        
        // Convert to miles (distance is in meters)
        float miles = distMeters * 0.000621371f;
        
        // Check if this day is in current week
        if (day >= weekStartDay && day <= weekEndDay) {
            int dayOfWeek = getDayOfWeek(day);
            weeklyMiles_[dayOfWeek] += miles;
            totalMiles_ += miles;
            totalRuns_ += count;
        }
    }
}

void StatsPanel::render(IDisplay* display) {
    display->fillScreen(0);
    
    uint16_t white = display->color565(255, 255, 255);
    uint16_t green = display->color565(0, 255, 0);
    uint16_t dimGreen = display->color565(0, 100, 0);
    uint16_t orange = display->color565(255, 150, 0);
    uint16_t gray = display->color565(80, 80, 80);
    uint16_t cyan = display->color565(0, 200, 255);
    uint16_t dimWhite = display->color565(120, 120, 120);
    
    // === LEFT SIDE: Weekly bar chart ===
    // 7 bars: width=3, gap=1, total=27px starting at X=2
    int barStartX = 2;
    int barBaseY = 22;  // Moved up to make room for day labels
    int barWidth = 3;
    int barGap = 1;
    int maxBarHeight = 14;  // Slightly shorter to fit labels
    
    // Find max miles for scaling
    float maxMiles = 1.0f;
    for (int i = 0; i < 7; i++) {
        if (weeklyMiles_[i] > maxMiles) maxMiles = weeklyMiles_[i];
    }
    
    // Draw bars
    for (int i = 0; i < 7; i++) {
        int x = barStartX + i * (barWidth + barGap);
        int height = (int)((weeklyMiles_[i] / maxMiles) * maxBarHeight);
        if (height < 1 && weeklyMiles_[i] > 0) height = 1;
        
        if (height > 0) {
            // Today's bar is orange, others are green
            uint16_t barColor = (i == todayDayOfWeek_) ? orange : green;
            display->fillRect(x, barBaseY - height, barWidth, height, barColor);
        }
        
        // Base line tick
        display->drawPixel(x + 1, barBaseY + 1, gray);
    }
    
    // Day labels: S M T W T F S (below bars)
    const char dayLabels[] = {'S', 'M', 'T', 'W', 'T', 'F', 'S'};
    char labelBuf[2] = {0, 0};
    display->setTextSize(1);
    for (int i = 0; i < 7; i++) {
        int x = barStartX + i * (barWidth + barGap);
        // Highlight today's label
        uint16_t labelColor = (i == todayDayOfWeek_) ? orange : gray;
        display->setTextColor(labelColor);
        display->setCursor(x, 25);
        labelBuf[0] = dayLabels[i];
        display->print(labelBuf);
    }
    
    // === RIGHT SIDE: Stats summary ===
    int rightX = 34;
    
    // Total miles - big number
    display->setTextSize(1);
    display->setTextColor(white);
    display->setCursor(rightX, 2);
    char milesStr[8];
    snprintf(milesStr, sizeof(milesStr), "%d", (int)totalMiles_);
    display->print(milesStr);
    
    // "mi" label
    display->setTextColor(gray);
    int milesWidth = strlen(milesStr) * 6;
    display->setCursor(rightX + milesWidth + 1, 2);
    display->print("mi");
    
    // Progress bar toward goal
    int progY = 12;
    int progWidth = 26;
    int progHeight = 3;
    float progress = totalMiles_ / goalMiles_;
    if (progress > 1.0f) progress = 1.0f;
    int filledWidth = (int)(progress * progWidth);
    
    // Background
    display->fillRect(rightX, progY, progWidth, progHeight, dimGreen);
    // Filled portion
    if (filledWidth > 0) {
        display->fillRect(rightX, progY, filledWidth, progHeight, green);
    }
    
    // Goal indicator at end
    display->drawPixel(rightX + progWidth - 1, progY - 1, white);
    display->drawPixel(rightX + progWidth - 1, progY + progHeight, white);
    
    // Goal label: "/20"
    display->setTextColor(dimWhite);
    display->setCursor(rightX, 17);
    display->print("/20");
    
    // Runs count
    display->setTextColor(cyan);
    display->setCursor(rightX + 20, 17);
    char runsStr[8];
    snprintf(runsStr, sizeof(runsStr), "%dr", totalRuns_);
    display->print(runsStr);
    
    // Week label at top (subtle)
    display->setTextColor(dimWhite);
    display->setCursor(2, 0);
    display->print("WEEK");
    
    // Animated pulse dot
    int pulsePhase = (animTime_ / 500) % 2;
    if (pulsePhase == 0) {
        display->drawPixel(61, 2, green);
    }
    
    display->show();
}
