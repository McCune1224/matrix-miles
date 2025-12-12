#include "MatrixRain.h"
#include <stdlib.h>  // for random()

MatrixRain::MatrixRain() 
    : active_(false)
    , elapsed_(0)
    , duration_(1500)
    , lastDropTime_(0) {
    // Initialize all drops as inactive
    for (int i = 0; i < RAIN_COLUMNS; i++) {
        drops_[i].headY = -10;  // Off screen
        drops_[i].speed = 0;
        drops_[i].length = 0;
        drops_[i].delay = 0;
    }
    // Clear trail buffer
    for (int i = 0; i < RAIN_COLUMNS * RAIN_HEIGHT; i++) {
        trailBuffer_[i] = 0;
    }
}

void MatrixRain::start(uint32_t durationMs) {
    active_ = true;
    elapsed_ = 0;
    duration_ = durationMs;
    lastDropTime_ = 0;
    
    // Clear trail buffer
    for (int i = 0; i < RAIN_COLUMNS * RAIN_HEIGHT; i++) {
        trailBuffer_[i] = 0;
    }
    
    // Initialize raindrops - OMEGA FAST, nearly instant start
    for (int i = 0; i < RAIN_COLUMNS; i++) {
        initDrop(i);
        // Minimal stagger - 0-1 cycles only
        drops_[i].delay = random(0, 2);
        drops_[i].headY = -random(0, 3);  // Start right at top
    }
}

void MatrixRain::initDrop(int col) {
    // Randomize drop properties - OMEGA FAST
    drops_[col].headY = -1;
    drops_[col].speed = 6 + random(0, 4);   // 6-9 pixels per update (OMEGA FAST)
    drops_[col].length = 2 + random(0, 3);  // 2-4 pixel trail (very short)
    drops_[col].delay = 0;
}

void MatrixRain::stop() {
    active_ = false;
}

bool MatrixRain::update(uint32_t deltaMs) {
    if (!active_) return true;
    
    elapsed_ += deltaMs;
    lastDropTime_ += deltaMs;
    
    // Update drops every ~10ms for OMEGA FAST animation
    if (lastDropTime_ >= 10) {
        lastDropTime_ = 0;
        
        // Fade existing trails very quickly
        for (int i = 0; i < RAIN_COLUMNS * RAIN_HEIGHT; i++) {
            if (trailBuffer_[i] > 80) {
                trailBuffer_[i] -= 80;  // OMEGA fast fade
            } else {
                trailBuffer_[i] = 0;
            }
        }
        
        // Update each raindrop
        for (int col = 0; col < RAIN_COLUMNS; col++) {
            RainDrop& drop = drops_[col];
            
            // Handle delay before starting
            if (drop.delay > 0) {
                drop.delay--;
                continue;
            }
            
            // Move the drop down - OMEGA FAST (4 pixels per update)
            drop.headY += 4;
            
            // If drop head is on screen, add bright pixel to trail
            if (drop.headY >= 0 && drop.headY < RAIN_HEIGHT) {
                trailBuffer_[drop.headY * RAIN_COLUMNS + col] = 255;  // Bright head
            }
            
            // Add trail behind head (slightly dimmer)
            for (int t = 1; t < drop.length && t <= drop.headY; t++) {
                int trailY = drop.headY - t;
                if (trailY >= 0 && trailY < RAIN_HEIGHT) {
                    int idx = trailY * RAIN_COLUMNS + col;
                    // Brightness decreases along trail
                    uint8_t brightness = 200 - (t * 180 / drop.length);
                    if (brightness > trailBuffer_[idx]) {
                        trailBuffer_[idx] = brightness;
                    }
                }
            }
            
            // Reset drop when it goes off screen
            if (drop.headY > RAIN_HEIGHT + drop.length) {
                initDrop(col);
                drop.headY = -random(0, 2);  // Instant restart
            }
        }
    }
    
    // Check if duration exceeded
    if (elapsed_ >= duration_) {
        active_ = false;
        return true;
    }
    
    return false;
}

void MatrixRain::render(IDisplay* display) {
    if (!active_) return;
    
    display->fillScreen(0);
    
    // Render trail buffer
    for (int y = 0; y < RAIN_HEIGHT; y++) {
        for (int x = 0; x < RAIN_COLUMNS; x++) {
            uint8_t brightness = trailBuffer_[y * RAIN_COLUMNS + x];
            if (brightness > 0) {
                display->drawPixel(x, y, getGreen(display, brightness));
            }
        }
    }
    
    // Draw bright white heads for active drops
    for (int col = 0; col < RAIN_COLUMNS; col++) {
        int headY = drops_[col].headY;
        if (headY >= 0 && headY < RAIN_HEIGHT && drops_[col].delay == 0) {
            // Head is bright white/light green
            display->drawPixel(col, headY, display->color565(200, 255, 200));
        }
    }
    
    display->show();
}

uint16_t MatrixRain::getGreen(IDisplay* display, uint8_t brightness) {
    // Green channel proportional to brightness
    // Add slight variation to green shades
    uint8_t g = brightness;
    uint8_t r = brightness / 8;  // Tiny bit of red for warmer look
    uint8_t b = brightness / 16; // Even less blue
    return display->color565(r, g, b);
}
