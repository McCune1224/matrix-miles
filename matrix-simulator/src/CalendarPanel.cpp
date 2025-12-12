#include "CalendarPanel.h"
#include "StravaClient.h"
#include <cstring>

CalendarPanel::CalendarPanel() 
    : month_(1), year_(2025), animTime_(0), showDiamond_(true) {
    // Default to current month
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    year_ = timeinfo->tm_year + 1900;
    month_ = timeinfo->tm_mon + 1;
}

void CalendarPanel::onEnter() {
    animTime_ = 0;
    showDiamond_ = true;
}

void CalendarPanel::setMonth(int month, int year) {
    month_ = month;
    year_ = year;
}

void CalendarPanel::setActivities(const std::vector<CalendarDay>& activities) {
    activities_ = activities;
}

void CalendarPanel::nextMonth() {
    month_++;
    if (month_ > 12) {
        month_ = 1;
        year_++;
    }
    refreshFromServer();
}

void CalendarPanel::prevMonth() {
    month_--;
    if (month_ < 1) {
        month_ = 12;
        year_--;
    }
    refreshFromServer();
}

void CalendarPanel::refreshFromServer() {
    if (stravaClient_) {
        activities_ = stravaClient_->fetchCalendarData(year_, month_);
    }
}

int CalendarPanel::getFirstDayOfWeek() const {
    struct tm timeStruct;
    memset(&timeStruct, 0, sizeof(struct tm));
    timeStruct.tm_year = year_ - 1900;
    timeStruct.tm_mon = month_ - 1;
    timeStruct.tm_mday = 1;
    timeStruct.tm_isdst = -1;
    mktime(&timeStruct);
    return timeStruct.tm_wday;  // 0=Sunday, 6=Saturday
}

int CalendarPanel::getDaysInMonth() const {
    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month_ == 2 && ((year_ % 4 == 0 && year_ % 100 != 0) || (year_ % 400 == 0))) {
        return 29;  // Leap year
    }
    return daysInMonth[month_];
}

bool CalendarPanel::hasActivity(int day) const {
    for (const auto& activity : activities_) {
        if (activity.day == day) {
            return true;
        }
    }
    return false;
}

void CalendarPanel::update(uint32_t deltaMs) {
    animTime_ += deltaMs;
    
    // Toggle shape every 500ms
    if (animTime_ >= 500) {
        animTime_ -= 500;
        showDiamond_ = !showDiamond_;
    }
}

void CalendarPanel::render(IDisplay* display) {
    display->fillScreen(0);  // Clear to black
    
    drawHeader(display);
    drawCalendarGrid(display);
}

void CalendarPanel::drawHeader(IDisplay* display) {
    uint16_t white = display->color565(255, 255, 255);
    
    // Draw day names manually: S M T W T F S
    // Each letter is 3-4 pixels wide x 4 pixels tall
    // (Ported directly from MatrixDisplay.cpp)
    
    // S (column 0)
    display->drawPixel(3, 0, white);
    display->drawPixel(4, 0, white);
    display->drawPixel(3, 1, white);
    display->drawPixel(4, 2, white);
    display->drawPixel(3, 3, white);
    display->drawPixel(4, 3, white);
    
    // M (column 1)
    display->drawPixel(12, 0, white);
    display->drawPixel(12, 1, white);
    display->drawPixel(12, 2, white);
    display->drawPixel(12, 3, white);
    display->drawPixel(13, 1, white);
    display->drawPixel(14, 0, white);
    display->drawPixel(14, 1, white);
    display->drawPixel(14, 2, white);
    display->drawPixel(14, 3, white);
    
    // T (column 2)
    display->drawPixel(21, 0, white);
    display->drawPixel(22, 0, white);
    display->drawPixel(23, 0, white);
    display->drawPixel(22, 1, white);
    display->drawPixel(22, 2, white);
    display->drawPixel(22, 3, white);
    
    // W (column 3)
    display->drawPixel(30, 0, white);
    display->drawPixel(30, 1, white);
    display->drawPixel(30, 2, white);
    display->drawPixel(30, 3, white);
    display->drawPixel(31, 2, white);
    display->drawPixel(32, 0, white);
    display->drawPixel(32, 1, white);
    display->drawPixel(32, 2, white);
    display->drawPixel(32, 3, white);
    
    // T (column 4)
    display->drawPixel(39, 0, white);
    display->drawPixel(40, 0, white);
    display->drawPixel(41, 0, white);
    display->drawPixel(40, 1, white);
    display->drawPixel(40, 2, white);
    display->drawPixel(40, 3, white);
    
    // F (column 5)
    display->drawPixel(48, 0, white);
    display->drawPixel(49, 0, white);
    display->drawPixel(50, 0, white);
    display->drawPixel(48, 1, white);
    display->drawPixel(49, 1, white);
    display->drawPixel(48, 2, white);
    display->drawPixel(48, 3, white);
    
    // S (column 6)
    display->drawPixel(57, 0, white);
    display->drawPixel(58, 0, white);
    display->drawPixel(57, 1, white);
    display->drawPixel(58, 2, white);
    display->drawPixel(57, 3, white);
    display->drawPixel(58, 3, white);
}

void CalendarPanel::drawDiamond(IDisplay* display, int x, int y, uint16_t color) {
    // Diamond pattern (rotated square, 4 pixels)
    display->drawPixel(x + 1, y, color);      // top
    display->drawPixel(x, y + 1, color);      // left
    display->drawPixel(x + 2, y + 1, color);  // right
    display->drawPixel(x + 1, y + 2, color);  // bottom
}

void CalendarPanel::drawSquare(IDisplay* display, int x, int y, uint16_t color) {
    // Filled 3x3 square
    display->fillRect(x, y, 3, 3, color);
}

void CalendarPanel::drawCalendarGrid(IDisplay* display) {
    uint16_t white = display->color565(255, 255, 255);
    uint16_t green = display->color565(0, 255, 0);
    
    int firstDayOfWeek = getFirstDayOfWeek();
    int numDaysInMonth = getDaysInMonth();
    
    int dayCounter = 1;
    
    for (int week = 0; week < 6; week++) {
        for (int dayOfWeek = 0; dayOfWeek < 7; dayOfWeek++) {
            // Skip days before month starts
            if (week == 0 && dayOfWeek < firstDayOfWeek) {
                continue;
            }
            
            // Stop if we've drawn all days
            if (dayCounter > numDaysInMonth) {
                return;
            }
            
            // Calculate diamond position
            int x = dayOfWeek * CELL_WIDTH + (CELL_WIDTH - DIAMOND_SIZE) / 2;
            int y = HEADER_HEIGHT + week * CELL_HEIGHT + (CELL_HEIGHT - DIAMOND_SIZE) / 2;
            
            bool activity = hasActivity(dayCounter);
            uint16_t color = activity ? green : white;
            
            // For activity days, animate between diamond and square
            if (activity && !showDiamond_) {
                drawSquare(display, x, y, color);
            } else {
                drawDiamond(display, x, y, color);
            }
            
            dayCounter++;
        }
    }
}
