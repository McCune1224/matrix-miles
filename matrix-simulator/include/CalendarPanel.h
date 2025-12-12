#ifndef CALENDAR_PANEL_H
#define CALENDAR_PANEL_H

#include "Panel.h"
#include "MockData.h"
#include <vector>
#include <ctime>

// Forward declaration
class StravaClient;

class CalendarPanel : public Panel {
public:
    CalendarPanel();
    
    void render(IDisplay* display) override;
    void update(uint32_t deltaMs) override;
    const char* name() const override { return "Calendar"; }
    
    void onEnter() override;
    
    // Set the month/year to display
    void setMonth(int month, int year);
    
    // Set activity data
    void setActivities(const std::vector<CalendarDay>& activities);
    
    // Set StravaClient for fetching data
    void setStravaClient(StravaClient* client) { stravaClient_ = client; }
    
    // Fetch activities from server for current month
    void refreshFromServer();
    
    // Navigate months
    void nextMonth();
    void prevMonth();
    
    // Get current month/year
    int getMonth() const { return month_; }
    int getYear() const { return year_; }
    
private:
    int month_;  // 1-12
    int year_;   // e.g., 2025
    
    std::vector<CalendarDay> activities_;
    StravaClient* stravaClient_ = nullptr;
    
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
    void drawDiamond(IDisplay* display, int x, int y, uint16_t color);
    void drawSquare(IDisplay* display, int x, int y, uint16_t color);
    
    int getFirstDayOfWeek() const;
    int getDaysInMonth() const;
    bool hasActivity(int day) const;
};

#endif // CALENDAR_PANEL_H
