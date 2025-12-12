#ifndef STATS_PANEL_H
#define STATS_PANEL_H

#include "Panel.h"
#include "StravaClient.h"

// StatsPanel - displays weekly/monthly activity statistics
// Shows activity count and total distance for week and month
class StatsPanel : public Panel {
public:
    StatsPanel();
    
    void render(IDisplay* display) override;
    void update(uint32_t deltaMs) override;
    const char* name() const override { return "Stats"; }
    
    void onEnter() override;
    
    // Set the current date for week calculations
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
    
    // Computed stats
    int weekActivities_;
    float weekDistance_;
    int monthActivities_;
    float monthDistance_;
    
    // Helper to compute stats from activity data
    void computeStats();
    
    // Get day of week (0=Sunday) for a given day
    int getDayOfWeek(int day) const;
};

#endif // STATS_PANEL_H
