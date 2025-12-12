#ifndef IDISPLAY_H
#define IDISPLAY_H

#include <Arduino.h>

// Display interface for panel rendering
// Abstracts the underlying display hardware (Protomatter, terminal, etc.)
class IDisplay {
public:
    virtual ~IDisplay() = default;
    
    // Core drawing methods
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void fillScreen(uint16_t color) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) = 0;
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) = 0;
    virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) = 0;
    
    // Color helper - converts RGB to 16-bit 565 format
    virtual uint16_t color565(uint8_t r, uint8_t g, uint8_t b) = 0;
    
    // Text rendering
    virtual void setCursor(int16_t x, int16_t y) = 0;
    virtual void setTextColor(uint16_t color) = 0;
    virtual void setTextSize(uint8_t size) = 0;
    virtual void print(const char* text) = 0;
    
    // Display update - push framebuffer to hardware
    virtual void show() = 0;
    
    // Dimensions
    virtual int16_t width() const = 0;
    virtual int16_t height() const = 0;
};

#endif // IDISPLAY_H
