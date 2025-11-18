#ifndef CALENDAR_DISPLAY_H
#define CALENDAR_DISPLAY_H

#include <Arduino.h>
#include <ArduinoJson.h>

class CalendarDisplay {
public:
  // Initialize the calendar display
  CalendarDisplay();
  
  // Print a calendar for the given year and month
  // output: Print stream to write to (e.g., Serial)
  // activityDays: array of day numbers (1-31) that have activities
  // activityCount: number of days with activities
  void printCalendar(Print& output, int year, int month, int* activityDays, int activityCount);
  
  // Parse JSON array of activities and extract days
  // Returns number of active days found
  int parseActivitiesFromJson(JsonArray activities, int* activityDays, int maxDays);
  
private:
  // Get the number of days in a given month/year
  int getDaysInMonth(int year, int month);
  
  // Get the day of week for the first day of the month (0=Sunday, 6=Saturday)
  int getFirstDayOfWeek(int year, int month);
  
  // Check if a year is a leap year
  bool isLeapYear(int year);
  
  // Check if a day has an activity
  bool hasDayActivity(int day, int* activityDays, int activityCount);
  
  // Get month name
  const char* getMonthName(int month);
};

#endif // CALENDAR_DISPLAY_H
