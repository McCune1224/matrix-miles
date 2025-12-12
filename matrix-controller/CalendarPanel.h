#ifndef CALENDAR_PANEL_H
#define CALENDAR_PANEL_H

#include "Panel.h"
#include "StravaClient.h"

// CalendarPanel - displays monthly calendar with activity markers
// Activity days are shown as green diamonds that animate between diamond/square
class CalendarPanel : public Panel {
public:
    CalendarPanel();
    
    void render(IDisplay* display) override;
    void update(uint32_t deltaMs) override;
    const char* name() const override { return "Calendar"; }
    
    void onEnter() override;
    
    // Set the month/year to display
    void setMonth(int month, int year);
    
    // Set the current day (for highlighting)
    void setCurrentDay(int day) { currentDay_ = day; }
    
    // Set activity data (copies data into internal array)
    void setActivities(const CalendarDay* activities, int count);
    
    // Get current month/year
    int getMonth() const { return month_; }
    int getYear() const { return year_; }
    
private:
    int month_;  // 1-12
    int year_;   // e.g., 2025
    int currentDay_;  // Current day of month (for display)
    
    // Activity data (max 31 days in a month)
    CalendarDay activities_[31];
    int activityCount_;
    
    // Animation state
    uint32_t animTime_;
    bool showDiamond_;  // Toggle between diamond and square for activity markers
    
    // Layout constants
    static constexpr int HEADER_HEIGHT = 5;
    static constexpr int CELL_WIDTH = 9;
    static constexpr int CELL_HEIGHT = 4;
    static constexpr int DIAMOND_SIZE = 3;
    
    // Helper methods
    void drawHeader(IDisplay* display);
    void drawCalendarGrid(IDisplay* display);
    void drawMonthYear(IDisplay* display);
    void drawDiamond(IDisplay* display, int16_t x, int16_t y, uint16_t color);
    void drawSquare(IDisplay* display, int16_t x, int16_t y, uint16_t color);
    
    int getFirstDayOfWeek() const;
    int getDaysInMonth() const;
    bool hasActivity(int day) const;
};

#endif // CALENDAR_PANEL_H
