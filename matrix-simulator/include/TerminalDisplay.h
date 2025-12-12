#ifndef TERMINAL_DISPLAY_H
#define TERMINAL_DISPLAY_H

#include <cstdint>
#include <string>

// Matrix dimensions (matching real hardware)
constexpr int MATRIX_WIDTH = 64;
constexpr int MATRIX_HEIGHT = 32;

// RGB color structure
struct Color {
    uint8_t r, g, b;
    
    Color() : r(0), g(0), b(0) {}
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
    
    bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b;
    }
    
    bool operator!=(const Color& other) const {
        return !(*this == other);
    }
};

// Convert 16-bit 565 color to RGB
inline Color color565ToRGB(uint16_t color) {
    uint8_t r = ((color >> 11) & 0x1F) << 3;
    uint8_t g = ((color >> 5) & 0x3F) << 2;
    uint8_t b = (color & 0x1F) << 3;
    return Color(r, g, b);
}

// Convert RGB to 16-bit 565 color
inline uint16_t rgbToColor565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

// Display interface - could be extended to support hardware in future
class IDisplay {
public:
    virtual ~IDisplay() = default;
    
    // Core drawing methods
    virtual void drawPixel(int x, int y, uint16_t color) = 0;
    virtual void fillScreen(uint16_t color) = 0;
    virtual void fillRect(int x, int y, int w, int h, uint16_t color) = 0;
    virtual void drawLine(int x0, int y0, int x1, int y1, uint16_t color) = 0;
    virtual void drawFastHLine(int x, int y, int w, uint16_t color) = 0;
    virtual void drawFastVLine(int x, int y, int h, uint16_t color) = 0;
    
    // Color helper
    virtual uint16_t color565(uint8_t r, uint8_t g, uint8_t b) = 0;
    
    // Text rendering
    virtual void setCursor(int x, int y) = 0;
    virtual void setTextColor(uint16_t color) = 0;
    virtual void setTextSize(int size) = 0;
    virtual void print(const char* text) = 0;
    virtual void print(const std::string& text) = 0;
    
    // Display update
    virtual void show() = 0;
    
    // Dimensions
    virtual int width() const = 0;
    virtual int height() const = 0;
};

// Terminal-based display using ANSI escape codes and half-block characters
class TerminalDisplay : public IDisplay {
public:
    TerminalDisplay();
    ~TerminalDisplay() override;
    
    // IDisplay implementation
    void drawPixel(int x, int y, uint16_t color) override;
    void fillScreen(uint16_t color) override;
    void fillRect(int x, int y, int w, int h, uint16_t color) override;
    void drawLine(int x0, int y0, int x1, int y1, uint16_t color) override;
    void drawFastHLine(int x, int y, int w, uint16_t color) override;
    void drawFastVLine(int x, int y, int h, uint16_t color) override;
    
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) override;
    
    void setCursor(int x, int y) override;
    void setTextColor(uint16_t color) override;
    void setTextSize(int size) override;
    void print(const char* text) override;
    void print(const std::string& text) override;
    
    void show() override;
    
    int width() const override { return MATRIX_WIDTH; }
    int height() const override { return MATRIX_HEIGHT; }
    
    // Terminal-specific methods
    void initTerminal();
    void restoreTerminal();
    void hideCursor();
    void showCursor();
    void clearTerminal();
    
    // Direct framebuffer access for transitions
    Color* getFramebuffer() { return framebuffer_; }
    const Color* getFramebuffer() const { return framebuffer_; }
    void setPixelDirect(int x, int y, const Color& color);
    Color getPixel(int x, int y) const;
    
private:
    Color framebuffer_[MATRIX_WIDTH * MATRIX_HEIGHT];
    
    // Text cursor state
    int cursorX_ = 0;
    int cursorY_ = 0;
    uint16_t textColor_ = 0xFFFF;  // White
    int textSize_ = 1;
    
    // Simple 3x5 font for basic text (similar to hardware)
    void drawChar(int x, int y, char c, uint16_t color);
    
    // Helper for Bresenham line algorithm
    void drawLineLow(int x0, int y0, int x1, int y1, uint16_t color);
    void drawLineHigh(int x0, int y0, int x1, int y1, uint16_t color);
};

#endif // TERMINAL_DISPLAY_H
