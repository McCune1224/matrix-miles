#ifndef PROTO_DISPLAY_H
#define PROTO_DISPLAY_H

#include "IDisplay.h"
#include <Adafruit_Protomatter.h>

// ProtoDisplay - IDisplay wrapper for Adafruit_Protomatter
// Delegates all drawing calls to the underlying Protomatter matrix
class ProtoDisplay : public IDisplay {
public:
    // Constructor takes reference to existing Protomatter instance
    ProtoDisplay(Adafruit_Protomatter& matrix) : matrix_(matrix) {}
    
    // Core drawing methods - delegate to Protomatter
    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        matrix_.drawPixel(x, y, color);
    }
    
    void fillScreen(uint16_t color) override {
        matrix_.fillScreen(color);
    }
    
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override {
        matrix_.fillRect(x, y, w, h, color);
    }
    
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override {
        matrix_.drawLine(x0, y0, x1, y1, color);
    }
    
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override {
        matrix_.drawFastHLine(x, y, w, color);
    }
    
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override {
        matrix_.drawFastVLine(x, y, h, color);
    }
    
    // Color helper
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) override {
        return matrix_.color565(r, g, b);
    }
    
    // Text rendering
    void setCursor(int16_t x, int16_t y) override {
        matrix_.setCursor(x, y);
    }
    
    void setTextColor(uint16_t color) override {
        matrix_.setTextColor(color);
    }
    
    void setTextSize(uint8_t size) override {
        matrix_.setTextSize(size);
    }
    
    void print(const char* text) override {
        matrix_.print(text);
    }
    
    // Display update
    void show() override {
        matrix_.show();
    }
    
    // Dimensions
    int16_t width() const override {
        return matrix_.width();
    }
    
    int16_t height() const override {
        return matrix_.height();
    }
    
private:
    Adafruit_Protomatter& matrix_;
};

#endif // PROTO_DISPLAY_H
