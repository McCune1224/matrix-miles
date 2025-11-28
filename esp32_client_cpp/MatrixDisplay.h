#ifndef MATRIX_DISPLAY_H
#define MATRIX_DISPLAY_H

#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 32

class Adafruit_Protomatter;
extern Adafruit_Protomatter matrix;

// Activity data (hardcoded for testing)
extern int activityDays[];
extern int activityCount;

void displayCalendar();
void loadingAnimation();

#endif // MATRIX_DISPLAY_H