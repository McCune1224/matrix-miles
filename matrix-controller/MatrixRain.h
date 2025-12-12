#ifndef MATRIX_RAIN_H
#define MATRIX_RAIN_H

#include "IDisplay.h"

// Matrix rain effect - digital rain like in "The Matrix" movie
// Used for transitions between panels
// Each column has a "raindrop" that falls and leaves a fading trail

// Number of columns on display (64 pixels wide)
#define RAIN_COLUMNS 64
#define RAIN_HEIGHT 32

class MatrixRain {
public:
    MatrixRain();
    
    // Start the rain effect
    // duration: how long the effect runs in ms
    // callback: called when effect completes (can be nullptr)
    void start(uint32_t durationMs = 1500);
    
    // Update rain state
    // Returns true when effect is complete
    bool update(uint32_t deltaMs);
    
    // Render current rain frame
    void render(IDisplay* display);
    
    // Check if rain is currently active
    bool isActive() const { return active_; }
    
    // Reset/stop the effect
    void stop();
    
private:
    bool active_;
    uint32_t elapsed_;
    uint32_t duration_;
    uint32_t lastDropTime_;
    
    // Per-column state
    struct RainDrop {
        int8_t headY;      // Y position of the bright head (-1 = not started)
        uint8_t speed;     // Pixels per 100ms (varies per column)
        uint8_t length;    // Trail length
        uint8_t delay;     // Delay before starting (in update cycles)
    };
    
    RainDrop drops_[RAIN_COLUMNS];
    
    // Trail brightness buffer (for fading effect)
    // Stores brightness level 0-255 for each pixel
    uint8_t trailBuffer_[RAIN_COLUMNS * RAIN_HEIGHT];
    
    // Initialize a raindrop for a column
    void initDrop(int col);
    
    // Get green color with brightness
    uint16_t getGreen(IDisplay* display, uint8_t brightness);
};

#endif // MATRIX_RAIN_H
