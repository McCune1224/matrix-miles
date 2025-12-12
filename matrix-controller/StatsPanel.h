#ifndef STATS_PANEL_H
#define STATS_PANEL_H

#include "Panel.h"
#include "StravaClient.h"

// StatsPanel - displays weekly bar chart and stats summary
// Left side: 7-day bar chart (Sun-Sat)
// Right side: Total miles, progress bar toward goal, run count
class StatsPanel : public Panel {
public:
    StatsPanel();
    
    void render(IDisplay* display) override;
    void update(uint32_t deltaMs) override;
    const char* name() const override { return "Stats"; }
    
    void onEnter() override;
    
    // Set the current date for week calculations and "today" highlight
    void setCurrentDate(int day, int month, int year);
    
    // Set activity data (same data as calendar)
    void setActivities(const CalendarDay* activities, int count);
    
private:
    int currentDay_;
    int month_;
    int year_;
    
    // Activity data (max 31 days)
    CalendarDay activities_[31];
    int activityCount_;
    
    // Animation
    uint32_t animTime_;
    
    // Weekly data for bar chart (Sun=0 to Sat=6)
    float weeklyMiles_[7];
    float totalMiles_;
    float goalMiles_;
    int totalRuns_;
    int todayDayOfWeek_;  // Which bar to highlight (0-6)
    
    // Helper to compute stats from activity data
    void computeStats();
    
    // Get day of week (0=Sunday) for a given day
    int getDayOfWeek(int day) const;
};

#endif // STATS_PANEL_H
