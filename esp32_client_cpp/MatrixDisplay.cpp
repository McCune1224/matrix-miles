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
   transitionToCalendar();
}

void displayCalendarWithMonth(int month, int year) {
     matrix.fillScreen(0);  // Clear screen to black

     // Colors
     uint16_t white = matrix.color565(255, 255, 255);
     uint16_t green = matrix.color565(0, 255, 0);
     
     // Layout calculations
     // Display: 64x32 pixels
     // Header: 5 pixels (4-pixel tall letters + 1 pixel gap)
     // Calendar: 6 rows for worst case months, (32-5)/6 = ~4.5px per row
     int headerHeight = 5;     // 4-pixel tall letters + 1 gap
     int cellWidth = 9;        // 64 / 7 ≈ 9 pixels per column
     int cellHeight = 4;       // (32 - 5) / 6 = ~4.5 pixels per row
     int diamondSize = 2;      // Diamond: 2x2 pixel core (appears 3x3 rotated)
     
     // === DRAW HEADER - CUSTOM 4-PIXEL TALL LETTERS ===
     // Draw day names manually: S M T W T F S
     // Each letter is 3-4 pixels wide x 4 pixels tall for better legibility
     
     // S (column 0) - more recognizable S shape
     matrix.drawPixel(3, 0, white);
     matrix.drawPixel(4, 0, white);
     matrix.drawPixel(3, 1, white);
     matrix.drawPixel(4, 2, white);
     matrix.drawPixel(3, 3, white);
     matrix.drawPixel(4, 3, white);
     
     // M (column 1) - vertical lines with peak
     matrix.drawPixel(12, 0, white);
     matrix.drawPixel(12, 1, white);
     matrix.drawPixel(12, 2, white);
     matrix.drawPixel(12, 3, white);
     matrix.drawPixel(13, 1, white); // peak
     matrix.drawPixel(14, 0, white);
     matrix.drawPixel(14, 1, white);
     matrix.drawPixel(14, 2, white);
     matrix.drawPixel(14, 3, white);
     
     // T (column 2) - clear T shape
     matrix.drawPixel(21, 0, white);
     matrix.drawPixel(22, 0, white);
     matrix.drawPixel(23, 0, white);
     matrix.drawPixel(22, 1, white);
     matrix.drawPixel(22, 2, white);
     matrix.drawPixel(22, 3, white);
     
     // W (column 3) - vertical lines with valley
     matrix.drawPixel(30, 0, white);
     matrix.drawPixel(30, 1, white);
     matrix.drawPixel(30, 2, white);
     matrix.drawPixel(30, 3, white);
     matrix.drawPixel(31, 2, white); // valley
     matrix.drawPixel(32, 0, white);
     matrix.drawPixel(32, 1, white);
     matrix.drawPixel(32, 2, white);
     matrix.drawPixel(32, 3, white);
     
     // T (column 4) - clear T shape
     matrix.drawPixel(39, 0, white);
     matrix.drawPixel(40, 0, white);
     matrix.drawPixel(41, 0, white);
     matrix.drawPixel(40, 1, white);
     matrix.drawPixel(40, 2, white);
     matrix.drawPixel(40, 3, white);
     
     // F (column 5) - clear F with horizontal bars
     matrix.drawPixel(48, 0, white);
     matrix.drawPixel(49, 0, white);
     matrix.drawPixel(50, 0, white);
     matrix.drawPixel(48, 1, white);
     matrix.drawPixel(49, 1, white);
     matrix.drawPixel(48, 2, white);
     matrix.drawPixel(48, 3, white);
     
     // S (column 6) - more recognizable S shape
     matrix.drawPixel(57, 0, white);
     matrix.drawPixel(58, 0, white);
     matrix.drawPixel(57, 1, white);
     matrix.drawPixel(58, 2, white);
     matrix.drawPixel(57, 3, white);
     matrix.drawPixel(58, 3, white);
    
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
        
        // Calculate diamond position (diamonds are 3x3 pixels when drawn)
        int diamondDisplaySize = 3;  // Actual visual size of diamond
        int x = dayOfWeek * cellWidth + (cellWidth - diamondDisplaySize) / 2;  // Center in column
        int y = headerHeight + week * cellHeight + (cellHeight - diamondDisplaySize) / 2;  // Center in row
        
        // Check if this day has activity
        bool hasActivity = false;
        for (int i = 0; i < activityCount; i++) {
          if (activityDays[i].day == dayCounter) {
            hasActivity = true;
            break;
          }
        }
        
        // Draw diamond (rotated square)
        uint16_t boxColor = hasActivity ? green : white;
        // 2x2 diamond pattern (4 pixels total)
        matrix.drawPixel(x + 1, y, boxColor);      // top
        matrix.drawPixel(x, y + 1, boxColor);      // left
        matrix.drawPixel(x + 2, y + 1, boxColor);  // right
        matrix.drawPixel(x + 1, y + 2, boxColor);  // bottom
        
        dayCounter++;
      }
      
      if (dayCounter > numDaysInMonth) {
        break;
      }
    }
     
    // Calendar is fully drawn to buffer - now show it with transition
    matrix.show();  // Display the calendar immediately
}

void transitionToCalendar() {
    // For now, just ensure calendar is visible
    // TODO: Add cool transition effect that doesn't overwrite calendar
    delay(100);
}

void animateCalendar() {
    // Animate green activity diamonds by rotating between diamond and square shapes
    // This runs continuously in the loop for visual flare
    
    static unsigned long lastAnimTime = 0;
    static bool showDiamond = true;
    
    // Animate every 500ms (2 FPS alternation between shapes)
    if (millis() - lastAnimTime < 500) {
      return;
    }
    
    lastAnimTime = millis();
    showDiamond = !showDiamond;  // Toggle shape
    
    // Only animate if we have activities to show
    if (activityCount == 0) {
      return;
    }
    
    // Get current month/year from extern variables
    extern int currentDisplayMonth;
    extern int currentDisplayYear;
    
    if (currentDisplayYear <= 1970) {
      return;  // No valid calendar displayed yet
    }
    
    // Colors
    uint16_t white = matrix.color565(255, 255, 255);
    uint16_t green = matrix.color565(0, 255, 0);
    uint16_t black = matrix.color565(0, 0, 0);
    
    // Layout calculations (must match displayCalendarWithMonth)
    int headerHeight = 5;
    int cellWidth = 9;
    int cellHeight = 4;
    int diamondDisplaySize = 3;
    
    // Calculate first day of month
    struct tm timeStruct;
    memset(&timeStruct, 0, sizeof(struct tm));
    timeStruct.tm_year = currentDisplayYear - 1900;
    timeStruct.tm_mon = currentDisplayMonth - 1;
    timeStruct.tm_mday = 1;
    timeStruct.tm_isdst = -1;
    mktime(&timeStruct);
    int firstDayOfWeek = timeStruct.tm_wday;
    
    // Get days in month
    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (currentDisplayMonth == 2 && ((currentDisplayYear % 4 == 0 && currentDisplayYear % 100 != 0) || (currentDisplayYear % 400 == 0))) {
      daysInMonth[2] = 29;
    }
    int numDaysInMonth = daysInMonth[currentDisplayMonth];
    
    // Redraw only the green activity diamonds with new shape
    int dayCounter = 1;
    
    for (int week = 0; week < 6; week++) {
      for (int dayOfWeek = 0; dayOfWeek < 7; dayOfWeek++) {
        if (week == 0 && dayOfWeek < firstDayOfWeek) {
          continue;
        }
        
        if (dayCounter > numDaysInMonth) {
          break;
        }
        
        // Check if this day has activity
        bool hasActivity = false;
        for (int i = 0; i < activityCount; i++) {
          if (activityDays[i].day == dayCounter) {
            hasActivity = true;
            break;
          }
        }
        
        // Only animate green activity markers
        if (hasActivity) {
          int x = dayOfWeek * cellWidth + (cellWidth - diamondDisplaySize) / 2;
          int y = headerHeight + week * cellHeight + (cellHeight - diamondDisplaySize) / 2;
          
          // Clear the 3x3 area first
          matrix.fillRect(x, y, diamondDisplaySize, diamondDisplaySize, black);
          
          if (showDiamond) {
            // Draw diamond pattern
            matrix.drawPixel(x + 1, y, green);      // top
            matrix.drawPixel(x, y + 1, green);      // left
            matrix.drawPixel(x + 2, y + 1, green);  // right
            matrix.drawPixel(x + 1, y + 2, green);  // bottom
          } else {
            // Draw square pattern (2x2 filled square)
            matrix.fillRect(x + 0, y + 0, 3, 3, green);
          }
        }
        
        dayCounter++;
      }
      
      if (dayCounter > numDaysInMonth) {
        break;
      }
    }
    
    matrix.show();
}

void showLoadingStatus(const char* statusMessage) {
     // Smooth rotating square with color gradient and "Loading..." text - ANIMATED!
     static float rotation = 0.0f;
     
     // Show animation for 1200ms (enough for a few dot cycles)
     unsigned long startTime = millis();
     while (millis() - startTime < 1200) {
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
       
       // Left side: rotating square
       int centerX = 10;  // Move square to left side
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
       
       // Right side: "Wait..." text with animated dots
       matrix.setTextSize(1);
       matrix.setTextColor(matrix.color565(255, 255, 255));
       matrix.setCursor(19, 12);  // Position to right of square
       matrix.print("Wait");
       
       // Animate dots (cycle through 0, 1, 2, 3 dots every 300ms)
       int dotCount = ((millis() - startTime) / 300) % 4;
       for (int i = 0; i < dotCount; i++) {
         matrix.print(".");
       }
       
       matrix.show();
       delay(10);  // ~100 FPS for buttery smooth animation
     }
}
