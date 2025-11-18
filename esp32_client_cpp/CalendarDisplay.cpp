#include "CalendarDisplay.h"

CalendarDisplay::CalendarDisplay() {
  // Constructor - nothing to initialize for now
}

const char* CalendarDisplay::getMonthName(int month) {
  const char* months[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
  };
  if (month >= 1 && month <= 12) {
    return months[month - 1];
  }
  return "Unknown";
}

bool CalendarDisplay::isLeapYear(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int CalendarDisplay::getDaysInMonth(int year, int month) {
  const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  if (month < 1 || month > 12) return 0;
  
  int days = daysInMonth[month - 1];
  
  // Add extra day for February in leap years
  if (month == 2 && isLeapYear(year)) {
    days = 29;
  }
  
  return days;
}

int CalendarDisplay::getFirstDayOfWeek(int year, int month) {
  // Zeller's Congruence algorithm to find day of week
  // Returns 0=Sunday, 1=Monday, ..., 6=Saturday
  
  int q = 1;  // day of month (we want the first day)
  int m = month;
  int y = year;
  
  // Adjust for Zeller's (March = 3, ..., December = 12, Jan/Feb are 13/14 of prev year)
  if (m < 3) {
    m += 12;
    y -= 1;
  }
  
  int K = y % 100;  // year of century
  int J = y / 100;  // zero-based century
  
  int h = (q + ((13 * (m + 1)) / 5) + K + (K / 4) + (J / 4) - (2 * J)) % 7;
  
  // Convert Zeller's output (0=Sat) to our format (0=Sun)
  int dayOfWeek = ((h + 6) % 7);
  
  return dayOfWeek;
}

bool CalendarDisplay::hasDayActivity(int day, int* activityDays, int activityCount) {
  for (int i = 0; i < activityCount; i++) {
    if (activityDays[i] == day) {
      return true;
    }
  }
  return false;
}

void CalendarDisplay::printCalendar(int year, int month, int* activityDays, int activityCount) {
  int daysInMonth = getDaysInMonth(year, month);
  int firstDay = getFirstDayOfWeek(year, month);
  
  // Print header
  Serial.println();
  Serial.print("   ");
  Serial.print(getMonthName(month));
  Serial.print(" ");
  Serial.println(year);
  Serial.println("Su Mo Tu We Th Fr Sa");
  
  // Print leading spaces for first week
  for (int i = 0; i < firstDay; i++) {
    Serial.print("   ");
  }
  
  // Print calendar days
  int currentDayOfWeek = firstDay;
  
  for (int day = 1; day <= daysInMonth; day++) {
    Serial.print(" ");
    
    // Print X for activity, . for no activity
    if (hasDayActivity(day, activityDays, activityCount)) {
      Serial.print("X");
    } else {
      Serial.print(".");
    }
    
    Serial.print(" ");
    
    currentDayOfWeek++;
    
    // New line after Saturday
    if (currentDayOfWeek > 6) {
      Serial.println();
      currentDayOfWeek = 0;
    }
  }
  
  // Final newline if needed
  if (currentDayOfWeek != 0) {
    Serial.println();
  }
  Serial.println();
}

int CalendarDisplay::parseActivitiesFromJson(JsonArray activities, int* activityDays, int maxDays) {
  int count = 0;
  
  for (JsonObject activity : activities) {
    if (count >= maxDays) break;
    
    // Extract the day from start_date (format: "YYYY-MM-DD" or ISO8601)
    const char* startDate = activity["start_date"];
    
    if (startDate != nullptr && strlen(startDate) >= 10) {
      // Parse day from "YYYY-MM-DD" format (day is at position 8-9)
      int day = (startDate[8] - '0') * 10 + (startDate[9] - '0');
      
      // Check if this day is already in the array
      bool alreadyExists = false;
      for (int i = 0; i < count; i++) {
        if (activityDays[i] == day) {
          alreadyExists = true;
          break;
        }
      }
      
      if (!alreadyExists) {
        activityDays[count] = day;
        count++;
      }
    }
  }
  
  return count;
}
