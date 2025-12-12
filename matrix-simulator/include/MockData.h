#ifndef MOCK_DATA_H
#define MOCK_DATA_H

#include <vector>
#include <cstdint>

// Activity data for a single day
struct CalendarDay {
    int day;           // Day of month (1-31)
    int activityCount; // Number of activities
    float distance;    // Total distance in km
    
    CalendarDay() : day(0), activityCount(0), distance(0.0f) {}
    CalendarDay(int d, int c, float dist) : day(d), activityCount(c), distance(dist) {}
};

// Generate mock activity data for testing
inline std::vector<CalendarDay> generateMockActivities() {
    std::vector<CalendarDay> activities;
    
    // Simulate a month with scattered activities
    // Days with activities: 2, 5, 7, 10, 12, 14, 15, 18, 21, 23, 25, 28
    activities.push_back(CalendarDay(2, 1, 5.2f));
    activities.push_back(CalendarDay(5, 1, 10.5f));
    activities.push_back(CalendarDay(7, 2, 15.0f));  // Two activities
    activities.push_back(CalendarDay(10, 1, 8.3f));
    activities.push_back(CalendarDay(12, 1, 6.7f));
    activities.push_back(CalendarDay(14, 1, 12.1f));
    activities.push_back(CalendarDay(15, 1, 4.5f));
    activities.push_back(CalendarDay(18, 1, 21.0f));  // Long run!
    activities.push_back(CalendarDay(21, 1, 7.8f));
    activities.push_back(CalendarDay(23, 1, 9.2f));
    activities.push_back(CalendarDay(25, 1, 11.4f));
    activities.push_back(CalendarDay(28, 1, 5.0f));
    
    return activities;
}

// Get a lighter mock dataset
inline std::vector<CalendarDay> generateLightMockActivities() {
    std::vector<CalendarDay> activities;
    
    // Just a few activities
    activities.push_back(CalendarDay(3, 1, 5.0f));
    activities.push_back(CalendarDay(10, 1, 8.0f));
    activities.push_back(CalendarDay(20, 1, 6.0f));
    
    return activities;
}

// Get empty activities (for testing empty calendar)
inline std::vector<CalendarDay> generateEmptyActivities() {
    return std::vector<CalendarDay>();
}

#endif // MOCK_DATA_H
