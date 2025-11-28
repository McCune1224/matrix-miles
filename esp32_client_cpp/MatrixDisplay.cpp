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
  matrix.fillScreen(0);  // Clear screen to black

  // Use default font (5x7)
  matrix.setFont(NULL);
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.color565(255, 255, 255));  // White text

  // Layout: 32 pixels height
  // Header: 6 pixels
  // Calendar: 3 rows x 7 cols, 7 pixels each = 21 pixels
  // Stats: 5 pixels at bottom

  int headerHeight = 6;
  int calendarRows = 3;
  int calendarHeight = 7;  // 3x7=21
  int cellWidth = MATRIX_WIDTH / 7;  // 9 pixels
  int statsHeight = MATRIX_HEIGHT - headerHeight - calendarRows * calendarHeight;  // 32-6-21=5

  // Draw header: Days of week
  const char* days[] = {"M", "T", "W", "T", "F", "S", "S"};
  for (int i = 0; i < 7; i++) {
    int x = i * cellWidth + 2;  // Center in cell
    matrix.setCursor(x, 0);
    matrix.print(days[i]);
  }

  // Draw calendar grid (3 rows for days 1-21)
  for (int week = 0; week < calendarRows; week++) {
    for (int dayOfWeek = 0; dayOfWeek < 7; dayOfWeek++) {
      int day = week * 7 + dayOfWeek + 1;
      if (day > 31) break;

      int x = dayOfWeek * cellWidth;
      int y = headerHeight + week * calendarHeight;

      // Check if this day has activity
      bool hasActivity = false;
      for (int i = 0; i < activityCount; i++) {
        if (activityDays[i] == day) {
          hasActivity = true;
          break;
        }
      }

      // Draw smaller squares
      if (hasActivity) {
        matrix.fillRect(x + 2, y + 2, cellWidth - 4, calendarHeight - 4, matrix.color565(255, 255, 255));  // White fill, smaller
      } else {
        // Draw border for days not ran
        matrix.drawRect(x + 2, y + 2, cellWidth - 4, calendarHeight - 4, matrix.color565(128, 128, 128));  // Gray border, smaller
      }
    }
  }

  // Draw hard line between calendar and stats
  int lineY = headerHeight + calendarRows * calendarHeight;
  matrix.drawFastHLine(0, lineY - 1, MATRIX_WIDTH, matrix.color565(255, 255, 255));  // White line above stats

  // Draw stats area (placeholder)
  matrix.setCursor(0, lineY);
  matrix.print("Stats");

  matrix.show();
}