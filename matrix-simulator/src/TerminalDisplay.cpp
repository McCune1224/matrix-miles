#include "TerminalDisplay.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

// Simple 3x5 bitmap font for characters (each char is 3 wide x 5 tall)
// Stored as 5 bytes per character, each byte represents one row (3 bits used)
// Format: bit 2 = left, bit 1 = center, bit 0 = right
static const uint8_t FONT_3X5[][5] = {
    // Space (32)
    {0b000, 0b000, 0b000, 0b000, 0b000},
    // ! (33)
    {0b010, 0b010, 0b010, 0b000, 0b010},
    // " (34)
    {0b101, 0b101, 0b000, 0b000, 0b000},
    // # - Z omitted for brevity, add as needed
};

// Letter glyphs for A-Z (simplified)
static const uint8_t FONT_LETTERS[][5] = {
    // A
    {0b010, 0b101, 0b111, 0b101, 0b101},
    // B
    {0b110, 0b101, 0b110, 0b101, 0b110},
    // C
    {0b011, 0b100, 0b100, 0b100, 0b011},
    // D
    {0b110, 0b101, 0b101, 0b101, 0b110},
    // E
    {0b111, 0b100, 0b110, 0b100, 0b111},
    // F
    {0b111, 0b100, 0b110, 0b100, 0b100},
    // G
    {0b011, 0b100, 0b101, 0b101, 0b011},
    // H
    {0b101, 0b101, 0b111, 0b101, 0b101},
    // I
    {0b111, 0b010, 0b010, 0b010, 0b111},
    // J
    {0b001, 0b001, 0b001, 0b101, 0b010},
    // K
    {0b101, 0b110, 0b100, 0b110, 0b101},
    // L
    {0b100, 0b100, 0b100, 0b100, 0b111},
    // M
    {0b101, 0b111, 0b111, 0b101, 0b101},
    // N
    {0b101, 0b111, 0b111, 0b111, 0b101},
    // O
    {0b010, 0b101, 0b101, 0b101, 0b010},
    // P
    {0b110, 0b101, 0b110, 0b100, 0b100},
    // Q
    {0b010, 0b101, 0b101, 0b110, 0b011},
    // R
    {0b110, 0b101, 0b110, 0b101, 0b101},
    // S
    {0b011, 0b100, 0b010, 0b001, 0b110},
    // T
    {0b111, 0b010, 0b010, 0b010, 0b010},
    // U
    {0b101, 0b101, 0b101, 0b101, 0b010},
    // V
    {0b101, 0b101, 0b101, 0b010, 0b010},
    // W
    {0b101, 0b101, 0b111, 0b111, 0b101},
    // X
    {0b101, 0b101, 0b010, 0b101, 0b101},
    // Y
    {0b101, 0b101, 0b010, 0b010, 0b010},
    // Z
    {0b111, 0b001, 0b010, 0b100, 0b111},
};

// Number glyphs 0-9
static const uint8_t FONT_NUMBERS[][5] = {
    // 0
    {0b111, 0b101, 0b101, 0b101, 0b111},
    // 1
    {0b010, 0b110, 0b010, 0b010, 0b111},
    // 2
    {0b111, 0b001, 0b111, 0b100, 0b111},
    // 3
    {0b111, 0b001, 0b111, 0b001, 0b111},
    // 4
    {0b101, 0b101, 0b111, 0b001, 0b001},
    // 5
    {0b111, 0b100, 0b111, 0b001, 0b111},
    // 6
    {0b111, 0b100, 0b111, 0b101, 0b111},
    // 7
    {0b111, 0b001, 0b001, 0b001, 0b001},
    // 8
    {0b111, 0b101, 0b111, 0b101, 0b111},
    // 9
    {0b111, 0b101, 0b111, 0b001, 0b111},
};

TerminalDisplay::TerminalDisplay() {
    fillScreen(0);
}

TerminalDisplay::~TerminalDisplay() {
    restoreTerminal();
}

void TerminalDisplay::initTerminal() {
    hideCursor();
    clearTerminal();
}

void TerminalDisplay::restoreTerminal() {
    showCursor();
    // Reset colors
    printf("\033[0m\n");
}

void TerminalDisplay::hideCursor() {
    printf("\033[?25l");
    fflush(stdout);
}

void TerminalDisplay::showCursor() {
    printf("\033[?25h");
    fflush(stdout);
}

void TerminalDisplay::clearTerminal() {
    printf("\033[2J\033[H");
    fflush(stdout);
}

uint16_t TerminalDisplay::color565(uint8_t r, uint8_t g, uint8_t b) {
    return rgbToColor565(r, g, b);
}

void TerminalDisplay::drawPixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) {
        return;
    }
    framebuffer_[y * MATRIX_WIDTH + x] = color565ToRGB(color);
}

void TerminalDisplay::setPixelDirect(int x, int y, const Color& color) {
    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) {
        return;
    }
    framebuffer_[y * MATRIX_WIDTH + x] = color;
}

Color TerminalDisplay::getPixel(int x, int y) const {
    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) {
        return Color(0, 0, 0);
    }
    return framebuffer_[y * MATRIX_WIDTH + x];
}

void TerminalDisplay::fillScreen(uint16_t color) {
    Color c = color565ToRGB(color);
    for (int i = 0; i < MATRIX_WIDTH * MATRIX_HEIGHT; i++) {
        framebuffer_[i] = c;
    }
}

void TerminalDisplay::fillRect(int x, int y, int w, int h, uint16_t color) {
    for (int py = y; py < y + h; py++) {
        for (int px = x; px < x + w; px++) {
            drawPixel(px, py, color);
        }
    }
}

void TerminalDisplay::drawFastHLine(int x, int y, int w, uint16_t color) {
    for (int i = 0; i < w; i++) {
        drawPixel(x + i, y, color);
    }
}

void TerminalDisplay::drawFastVLine(int x, int y, int h, uint16_t color) {
    for (int i = 0; i < h; i++) {
        drawPixel(x, y + i, color);
    }
}

// Bresenham's line algorithm
void TerminalDisplay::drawLineLow(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    int yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }
    int D = (2 * dy) - dx;
    int y = y0;
    
    for (int x = x0; x <= x1; x++) {
        drawPixel(x, y, color);
        if (D > 0) {
            y += yi;
            D += 2 * (dy - dx);
        } else {
            D += 2 * dy;
        }
    }
}

void TerminalDisplay::drawLineHigh(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    int xi = 1;
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }
    int D = (2 * dx) - dy;
    int x = x0;
    
    for (int y = y0; y <= y1; y++) {
        drawPixel(x, y, color);
        if (D > 0) {
            x += xi;
            D += 2 * (dx - dy);
        } else {
            D += 2 * dx;
        }
    }
}

void TerminalDisplay::drawLine(int x0, int y0, int x1, int y1, uint16_t color) {
    if (std::abs(y1 - y0) < std::abs(x1 - x0)) {
        if (x0 > x1) {
            drawLineLow(x1, y1, x0, y0, color);
        } else {
            drawLineLow(x0, y0, x1, y1, color);
        }
    } else {
        if (y0 > y1) {
            drawLineHigh(x1, y1, x0, y0, color);
        } else {
            drawLineHigh(x0, y0, x1, y1, color);
        }
    }
}

void TerminalDisplay::setCursor(int x, int y) {
    cursorX_ = x;
    cursorY_ = y;
}

void TerminalDisplay::setTextColor(uint16_t color) {
    textColor_ = color;
}

void TerminalDisplay::setTextSize(int size) {
    textSize_ = size;
}

void TerminalDisplay::drawChar(int x, int y, char c, uint16_t color) {
    const uint8_t* glyph = nullptr;
    
    if (c >= 'A' && c <= 'Z') {
        glyph = FONT_LETTERS[c - 'A'];
    } else if (c >= 'a' && c <= 'z') {
        glyph = FONT_LETTERS[c - 'a'];
    } else if (c >= '0' && c <= '9') {
        glyph = FONT_NUMBERS[c - '0'];
    } else if (c == '.') {
        // Draw a single dot at bottom
        drawPixel(x + 1, y + 4, color);
        return;
    } else if (c == ' ') {
        return;  // Space is just empty
    } else {
        return;  // Unknown character
    }
    
    if (glyph) {
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 3; col++) {
                if (glyph[row] & (0b100 >> col)) {
                    for (int sy = 0; sy < textSize_; sy++) {
                        for (int sx = 0; sx < textSize_; sx++) {
                            drawPixel(x + col * textSize_ + sx, 
                                     y + row * textSize_ + sy, color);
                        }
                    }
                }
            }
        }
    }
}

void TerminalDisplay::print(const char* text) {
    while (*text) {
        if (*text == '\n') {
            cursorX_ = 0;
            cursorY_ += 6 * textSize_;
        } else {
            drawChar(cursorX_, cursorY_, *text, textColor_);
            cursorX_ += 4 * textSize_;  // 3 pixels + 1 spacing
        }
        text++;
    }
}

void TerminalDisplay::print(const std::string& text) {
    print(text.c_str());
}

void TerminalDisplay::show() {
    // Move cursor to top-left
    printf("\033[H");
    
    // Draw top border
    printf("\033[90m+");
    for (int x = 0; x < MATRIX_WIDTH; x++) {
        printf("-");
    }
    printf("+\033[0m\n");
    
    // Render framebuffer using half-block characters
    // Each terminal row represents 2 pixel rows
    // Upper half-block: ▀ (U+2580) - top pixel
    // Lower half-block: ▄ (U+2584) - bottom pixel
    // Full block: █ (U+2588) - both pixels same color
    // Space: both pixels black
    
    for (int y = 0; y < MATRIX_HEIGHT; y += 2) {
        printf("\033[90m|\033[0m");  // Left border
        
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            Color top = framebuffer_[y * MATRIX_WIDTH + x];
            Color bottom = (y + 1 < MATRIX_HEIGHT) 
                ? framebuffer_[(y + 1) * MATRIX_WIDTH + x]
                : Color(0, 0, 0);
            
            bool topBlack = (top.r < 10 && top.g < 10 && top.b < 10);
            bool bottomBlack = (bottom.r < 10 && bottom.g < 10 && bottom.b < 10);
            
            if (topBlack && bottomBlack) {
                // Both black - just space
                printf(" ");
            } else if (top == bottom) {
                // Same color - full block
                printf("\033[38;2;%d;%d;%dm█\033[0m", top.r, top.g, top.b);
            } else if (bottomBlack) {
                // Only top pixel - upper half block
                printf("\033[38;2;%d;%d;%dm▀\033[0m", top.r, top.g, top.b);
            } else if (topBlack) {
                // Only bottom pixel - lower half block
                printf("\033[38;2;%d;%d;%dm▄\033[0m", bottom.r, bottom.g, bottom.b);
            } else {
                // Both different colors - upper half with fg=top, bg=bottom
                printf("\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm▀\033[0m",
                       top.r, top.g, top.b,
                       bottom.r, bottom.g, bottom.b);
            }
        }
        
        printf("\033[90m|\033[0m\n");  // Right border
    }
    
    // Draw bottom border
    printf("\033[90m+");
    for (int x = 0; x < MATRIX_WIDTH; x++) {
        printf("-");
    }
    printf("+\033[0m\n");
    
    fflush(stdout);
}
