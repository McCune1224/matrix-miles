#include "MatrixDisplay.h"
#include <Arduino.h>
#include <Adafruit_Protomatter.h>

extern Adafruit_Protomatter matrix;

void horizontalScan() {
  matrix.fillScreen(0);
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    if (y > 0) matrix.drawFastHLine(0, y-1, MATRIX_WIDTH, 0); // erase previous
    matrix.drawFastHLine(0, y, MATRIX_WIDTH, matrix.color565(0, 255, 0)); // green line
    matrix.show();
    delay(30);
  }
  matrix.fillScreen(0); // clear
}

void verticalScan() {
  matrix.fillScreen(0);
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    if (x > 0) matrix.drawFastVLine(x-1, 0, MATRIX_HEIGHT, 0); // erase previous
    matrix.drawFastVLine(x, 0, MATRIX_HEIGHT, matrix.color565(0, 255, 0)); // green line
    matrix.show();
    delay(20);
  }
  matrix.fillScreen(0); // clear
}

void loadingAnimation() {
  int anim = random(2); // 0 or 1
  if (anim == 0) {
    horizontalScan();
  } else {
    verticalScan();
  }
}

void displayCalendar() {
  displayCalendarWithMonth(11, 2024);  // Default to current month (update this)
}

void displayCalendarWithMonth(int month, int year) {
  matrix.fillScreen(0);  // Clear screen to black

  // Use default font (5x7)
  matrix.setFont(NULL);
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.color565(255, 255, 255));  // White text

  // Month names abbreviated (3 chars max)
  const char* monthNames[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  // Layout: 32 pixels height
  // Header: 2 rows (month/year + day names) = 14 pixels
  // Calendar: 3 rows x 7 cols, 6 pixels each = 18 pixels
  // Total: 32 pixels

  int headerHeight = 14;
  int calendarRows = 3;
  int calendarHeight = 6;
  int cellWidth = MATRIX_WIDTH / 7;  // ~9 pixels

  // Draw month/year header
  matrix.setTextSize(1);
  matrix.setCursor(0, 0);
  char monthYearStr[12];
  sprintf(monthYearStr, "%s %d", monthNames[month], year);
  matrix.print(monthYearStr);

  // Draw day of week abbreviations
  const char* dayNames[] = {"M", "T", "W", "T", "F", "S", "S"};
  for (int i = 0; i < 7; i++) {
    int x = i * cellWidth + 2;
    matrix.setCursor(x, 8);
    matrix.print(dayNames[i]);
  }

  // Draw calendar grid
  for (int week = 0; week < calendarRows; week++) {
    for (int dayOfWeek = 0; dayOfWeek < 7; dayOfWeek++) {
      int day = week * 7 + dayOfWeek + 1;
      if (day > 31) break;

      int x = dayOfWeek * cellWidth;
      int y = headerHeight + week * calendarHeight;

      // Check if this day has activity
      bool hasActivity = false;
      for (int i = 0; i < activityCount; i++) {
        if (activityDays[i].day == day) {
          hasActivity = true;
          break;
        }
      }

      // Draw smaller squares
      if (hasActivity) {
        // White fill for days with activity
        matrix.fillRect(x + 1, y + 1, cellWidth - 2, calendarHeight - 2, matrix.color565(255, 255, 255));
      } else {
        // Gray border for days without activity
        matrix.drawRect(x + 1, y + 1, cellWidth - 2, calendarHeight - 2, matrix.color565(80, 80, 80));
      }
    }
  }

  matrix.show();
}