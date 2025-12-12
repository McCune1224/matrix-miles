#include "CalendarPanel.h"
#include <ctime>
#include <cstring>
#include <cstdio>

CalendarPanel::CalendarPanel()
    : month_(1)
    , year_(1970)
    , currentDay_(1)
    , activityCount_(0)
    , animTime_(0)
    , showDiamond_(true) {
    memset(activities_, 0, sizeof(activities_));
}

void CalendarPanel::onEnter() {
    // Reset animation state when entering panel
    animTime_ = 0;
    showDiamond_ = true;
}

void CalendarPanel::setMonth(int month, int year) {
    month_ = month;
    year_ = year;
}

void CalendarPanel::setActivities(const CalendarDay* activities, int count) {
    if (count > 31) count = 31;
    activityCount_ = count;
    for (int i = 0; i < count; i++) {
        activities_[i] = activities[i];
    }
}

void CalendarPanel::update(uint32_t deltaMs) {
    animTime_ += deltaMs;
    
    // Toggle shape every 500ms
    if (animTime_ >= 500) {
        animTime_ = 0;
        showDiamond_ = !showDiamond_;
    }
}

void CalendarPanel::render(IDisplay* display) {
    display->fillScreen(0);
    drawHeader(display);
    drawCalendarGrid(display);
    drawMonthYear(display);
    display->show();
}

void CalendarPanel::drawHeader(IDisplay* display) {
    uint16_t red = display->color565(255, 0, 0);
    
    // Draw day names: S M T W T F S
    // Custom 4-pixel tall letters for better legibility on small display
    
    // S (column 0)
    display->drawPixel(3, 0, red);
    display->drawPixel(4, 0, red);
    display->drawPixel(3, 1, red);
    display->drawPixel(4, 2, red);
    display->drawPixel(3, 3, red);
    display->drawPixel(4, 3, red);
    
    // M (column 1)
    display->drawPixel(12, 0, red);
    display->drawPixel(12, 1, red);
    display->drawPixel(12, 2, red);
    display->drawPixel(12, 3, red);
    display->drawPixel(13, 1, red);
    display->drawPixel(14, 0, red);
    display->drawPixel(14, 1, red);
    display->drawPixel(14, 2, red);
    display->drawPixel(14, 3, red);
    
    // T (column 2)
    display->drawPixel(21, 0, red);
    display->drawPixel(22, 0, red);
    display->drawPixel(23, 0, red);
    display->drawPixel(22, 1, red);
    display->drawPixel(22, 2, red);
    display->drawPixel(22, 3, red);
    
    // W (column 3)
    display->drawPixel(30, 0, red);
    display->drawPixel(30, 1, red);
    display->drawPixel(30, 2, red);
    display->drawPixel(30, 3, red);
    display->drawPixel(31, 2, red);
    display->drawPixel(32, 0, red);
    display->drawPixel(32, 1, red);
    display->drawPixel(32, 2, red);
    display->drawPixel(32, 3, red);
    
    // T (column 4)
    display->drawPixel(39, 0, red);
    display->drawPixel(40, 0, red);
    display->drawPixel(41, 0, red);
    display->drawPixel(40, 1, red);
    display->drawPixel(40, 2, red);
    display->drawPixel(40, 3, red);
    
    // F (column 5)
    display->drawPixel(48, 0, red);
    display->drawPixel(49, 0, red);
    display->drawPixel(50, 0, red);
    display->drawPixel(48, 1, red);
    display->drawPixel(49, 1, red);
    display->drawPixel(48, 2, red);
    display->drawPixel(48, 3, red);
    
    // S (column 6)
    display->drawPixel(57, 0, red);
    display->drawPixel(58, 0, red);
    display->drawPixel(57, 1, red);
    display->drawPixel(58, 2, red);
    display->drawPixel(57, 3, red);
    display->drawPixel(58, 3, red);
}

void CalendarPanel::drawCalendarGrid(IDisplay* display) {
    if (year_ <= 1970) return;  // Invalid date
    
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
                break;
            }
            
            // Calculate position (centered in cell)
            int x = dayOfWeek * CELL_WIDTH + (CELL_WIDTH - DIAMOND_SIZE) / 2;
            int y = HEADER_HEIGHT + week * CELL_HEIGHT + (CELL_HEIGHT - DIAMOND_SIZE) / 2;
            
            // Check if this day has activity
            bool hasAct = hasActivity(dayCounter);
            uint16_t color = hasAct ? green : white;
            
            // Draw marker (animate green ones)
            if (hasAct && !showDiamond_) {
                drawSquare(display, x, y, color);
            } else {
                drawDiamond(display, x, y, color);
            }
            
            dayCounter++;
        }
        
        if (dayCounter > numDaysInMonth) {
            break;
        }
    }
}

void CalendarPanel::drawDiamond(IDisplay* display, int16_t x, int16_t y, uint16_t color) {
    // 3x3 diamond pattern
    display->drawPixel(x + 1, y, color);      // top
    display->drawPixel(x, y + 1, color);      // left
    display->drawPixel(x + 2, y + 1, color);  // right
    display->drawPixel(x + 1, y + 2, color);  // bottom
}

void CalendarPanel::drawSquare(IDisplay* display, int16_t x, int16_t y, uint16_t color) {
    // 3x3 filled square
    display->fillRect(x, y, 3, 3, color);
}

int CalendarPanel::getFirstDayOfWeek() const {
    struct tm timeStruct;
    memset(&timeStruct, 0, sizeof(struct tm));
    timeStruct.tm_year = year_ - 1900;
    timeStruct.tm_mon = month_ - 1;
    timeStruct.tm_mday = 1;
    timeStruct.tm_isdst = -1;
    mktime(&timeStruct);
    
    // tm_wday: 0=Sunday, 1=Monday, ..., 6=Saturday
    return timeStruct.tm_wday;
}

int CalendarPanel::getDaysInMonth() const {
    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // Leap year check
    if (month_ == 2) {
        if ((year_ % 4 == 0 && year_ % 100 != 0) || (year_ % 400 == 0)) {
            return 29;
        }
    }
    
    return daysInMonth[month_];
}

bool CalendarPanel::hasActivity(int day) const {
    for (int i = 0; i < activityCount_; i++) {
        if (activities_[i].day == day) {
            return true;
        }
    }
    return false;
}

void CalendarPanel::drawMonthYear(IDisplay* display) {
    if (year_ <= 1970) return;  // Invalid date
    
    // Format: MM/DD/YY (8 chars = 48 pixels wide at size 1)
    // Position in bottom area
    // Display is 64 wide, text at size 1 is 6px per char
    
    char dateStr[12];
    int shortYear = year_ % 100;  // Get last 2 digits of year
    snprintf(dateStr, sizeof(dateStr), "%02d/%02d/%02d", month_, currentDay_, shortYear);
    
    // 8 chars * 6px = 48px, center it: (64 - 48) / 2 = 8
    uint16_t gray = display->color565(100, 100, 100);
    display->setTextSize(1);
    display->setTextColor(gray);
    display->setCursor(8, 24);
    display->print(dateStr);
}
