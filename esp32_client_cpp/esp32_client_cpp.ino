#include <Arduino.h>
#include <Adafruit_Protomatter.h>
#include "MatrixDisplay.h"

#define USE_SERIAL Serial

// Matrix configuration for MatrixPortal M4 (64x32 RGB matrix)
#define MATRIX_CHAIN 1  // Number of matrix panels chained

// Matrix pins for MatrixPortal M4
uint8_t rgbPins[] = {7, 8, 9, 10, 11, 12};
uint8_t addrPins[] = {17, 18, 19, 20};
uint8_t clockPin = 14;
uint8_t latchPin = 15;
uint8_t oePin = 16;

// Create matrix object
Adafruit_Protomatter matrix(
  MATRIX_WIDTH, MATRIX_CHAIN,
  1, rgbPins,
  sizeof(addrPins), addrPins,
  clockPin, latchPin, oePin,
  true  // double buffering
);

// Activity data (hardcoded for testing)
int activityDays[31] = {1, 5, 10, 15, 20, 25};  // Days with runs
int activityCount = 6;

void setup() {
  USE_SERIAL.begin(115200);
  USE_SERIAL.println("Matrix Miles - Calendar Display Test");

  randomSeed(analogRead(0));  // For random animations

  // Initialize matrix
  ProtomatterStatus status = matrix.begin();
  USE_SERIAL.printf("Matrix status: %d\n", status);

  // Loading animation
  loadingAnimation();

  // Display calendar
  displayCalendar();
}

void loop() {
  // Do nothing, just display the calendar
  delay(1000);
}