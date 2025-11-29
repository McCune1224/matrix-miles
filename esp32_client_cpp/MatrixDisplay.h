#ifndef MATRIX_DISPLAY_H
#define MATRIX_DISPLAY_H

#include <Adafruit_Protomatter.h>
#include "StravaClient.h"

#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 32

extern Adafruit_Protomatter matrix;

// Activity data
extern CalendarDay activityDays[];
extern int activityCount;

void displayCalendar();
void displayCalendarWithMonth(int month, int year);
void loadingAnimation();

#endif // MATRIX_DISPLAY_H