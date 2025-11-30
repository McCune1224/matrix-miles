#include "MatrixDisplay.h"
#include <Arduino.h>
#include <Adafruit_Protomatter.h>
#include <ctime>
#include <cmath>

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
   // Try to get synced date from server first, fall back to system time
   int day = 1, month = 1, year = 1970;
   
   if (stravaClient != nullptr) {
     stravaClient->getSyncedDate(day, month, year);
   }
   
   // If we still have invalid date (1970), try system time as fallback
   if (year <= 1970) {
     time_t now = time(nullptr);
     struct tm* timeinfo = localtime(&now);
     year = timeinfo->tm_year + 1900;
     month = timeinfo->tm_mon + 1;
     day = timeinfo->tm_mday;
   }
   
   displayCalendarWithMonth(month, year);
}

void displayCalendarWithMonth(int month, int year) {
     matrix.fillScreen(0);  // Clear screen to black

     // Colors
     uint16_t white = matrix.color565(255, 255, 255);
     uint16_t green = matrix.color565(0, 255, 0);
     
     // Layout calculations
     int headerHeight = 2;     // Minimal header (just day names)
     int cellWidth = 9;        // 64 / 7 ≈ 9 pixels per column
     int cellHeight = 5;       // (32 - 2) / 6 ≈ 5 pixels per row (more space!)
     int boxSize = 3;          // Box size: 3x3 pixels
     
     // === DRAW HEADER ===
     matrix.setFont(NULL);
     matrix.setTextSize(0);    // Smaller font for more space
     matrix.setTextColor(white);
     
     // Day names (S M T W T F S) - starts with Sunday
     const char* dayNames[] = {"S", "M", "T", "W", "T", "F", "S"};
     for (int i = 0; i < 7; i++) {
       matrix.setCursor(i * cellWidth + 3, 0);  // Better centering in each column
       matrix.print(dayNames[i]);
     }
     
     // No separator line - maximizes space for calendar grid
    
    // === CALCULATE FIRST DAY OF MONTH ===
    struct tm timeStruct;
    memset(&timeStruct, 0, sizeof(struct tm));
    timeStruct.tm_year = year - 1900;
    timeStruct.tm_mon = month - 1;
    timeStruct.tm_mday = 1;
    timeStruct.tm_isdst = -1;
    mktime(&timeStruct);
    
    // tm_wday: 0=Sunday, 1=Monday, ..., 6=Saturday (perfect for our grid!)
    int firstDayOfWeek = timeStruct.tm_wday;
    
    // === GET DAYS IN MONTH ===
    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
      daysInMonth[2] = 29;  // Leap year
    }
    int numDaysInMonth = daysInMonth[month];
    
    // === DRAW CALENDAR GRID ===
    int dayCounter = 1;
    int dayOfWeekCounter = firstDayOfWeek;
    
    for (int week = 0; week < 6; week++) {
      for (int dayOfWeek = 0; dayOfWeek < 7; dayOfWeek++) {
        // Skip days before month starts (first week only)
        if (week == 0 && dayOfWeek < firstDayOfWeek) {
          continue;
        }
        
        // Stop if we've drawn all days
        if (dayCounter > numDaysInMonth) {
          break;
        }
        
        // Calculate box position
        int x = dayOfWeek * cellWidth + (cellWidth - boxSize) / 2;  // Center in column
        int y = headerHeight + week * cellHeight + (cellHeight - boxSize) / 2;  // Center in row
        
        // Check if this day has activity
        bool hasActivity = false;
        for (int i = 0; i < activityCount; i++) {
          if (activityDays[i].day == dayCounter) {
            hasActivity = true;
            break;
          }
        }
        
        // Draw bordered square
        uint16_t boxColor = hasActivity ? green : white;
        matrix.drawRect(x, y, boxSize, boxSize, boxColor);
        
        dayCounter++;
      }
      
      if (dayCounter > numDaysInMonth) {
        break;
      }
    }
    
    matrix.show();
}

void showLoadingStatus(const char* statusMessage) {
     // Smooth rotating square with color gradient - ANIMATED!
     static float rotation = 0.0f;
     
     // Show animation for 400ms (buttery smooth spinning)
     unsigned long startTime = millis();
     while (millis() - startTime < 400) {
       // Clear screen for this frame
       matrix.fillScreen(0);
       
       // Update rotation for smooth animation
       rotation += 9.0f;  // 9 degrees per frame = fast, smooth rotation
       if (rotation >= 360.0f) {
         rotation -= 360.0f;
       }
       
       // Color gradient cycling
       unsigned long elapsed = millis() % 3000;
       float huePhase = (elapsed / 3000.0f) * 6.0f;
       
       uint16_t colors[] = {
         matrix.color565(255, 0, 0),     // Red
         matrix.color565(255, 255, 0),   // Yellow
         matrix.color565(0, 255, 0),     // Green
         matrix.color565(0, 255, 255),   // Cyan
         matrix.color565(0, 0, 255),     // Blue
         matrix.color565(255, 0, 255)    // Magenta
       };
       int colorIdx = (int)huePhase % 6;
       uint16_t squareColor = colors[colorIdx];
       
       // Center of matrix
       int centerX = MATRIX_WIDTH / 2;
       int centerY = MATRIX_HEIGHT / 2;
       int squareSize = 6;
       int halfSize = squareSize / 2;
       
       // Calculate rotated corners
       float rad = rotation * 3.14159f / 180.0f;
       float cosRot = cosf(rad);
       float sinRot = sinf(rad);
       
       int corners[4][2] = {
         {-halfSize, -halfSize},  // Top-left
         {halfSize, -halfSize},   // Top-right
         {halfSize, halfSize},    // Bottom-right
         {-halfSize, halfSize}    // Bottom-left
       };
       
       int rotatedCorners[4][2];
       for (int i = 0; i < 4; i++) {
         float x = corners[i][0];
         float y = corners[i][1];
         rotatedCorners[i][0] = centerX + (int)(x * cosRot - y * sinRot);
         rotatedCorners[i][1] = centerY + (int)(x * sinRot + y * cosRot);
       }
       
       // Draw the rotated square outline
       for (int i = 0; i < 4; i++) {
         int next = (i + 1) % 4;
         matrix.drawLine(
           rotatedCorners[i][0], rotatedCorners[i][1],
           rotatedCorners[next][0], rotatedCorners[next][1],
           squareColor
         );
       }
       
       matrix.show();
       delay(10);  // ~100 FPS for buttery smooth animation
     }
}
