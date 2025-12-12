#include "Transition.h"
#include <stdlib.h>
#include <cmath>

Transition::Transition()
    : active_(false)
    , currentType_(TransitionType::PixelDissolve)
    , elapsed_(0)
    , duration_(400)
    , dissolveIndex_(0) {
    // Initialize dissolve order array
    for (int i = 0; i < TRANS_WIDTH * TRANS_HEIGHT; i++) {
        dissolveOrder_[i] = i;
    }
}

void Transition::start(TransitionType type, uint32_t durationMs) {
    currentType_ = type;
    duration_ = durationMs;
    elapsed_ = 0;
    active_ = true;
    dissolveIndex_ = 0;
    
    // Shuffle dissolve order for random pixel reveal
    if (type == TransitionType::PixelDissolve) {
        shuffleDissolveOrder();
    }
}

void Transition::stop() {
    active_ = false;
}

float Transition::getProgress() const {
    if (duration_ == 0) return 1.0f;
    float p = static_cast<float>(elapsed_) / static_cast<float>(duration_);
    return p > 1.0f ? 1.0f : p;
}

float Transition::easeInOut(float t) const {
    // Smooth ease-in-out curve
    return t < 0.5f 
        ? 2.0f * t * t 
        : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

void Transition::shuffleDissolveOrder() {
    // Fisher-Yates shuffle
    int n = TRANS_WIDTH * TRANS_HEIGHT;
    for (int i = n - 1; i > 0; i--) {
        int j = random(0, i + 1);
        uint8_t temp = dissolveOrder_[i];
        dissolveOrder_[i] = dissolveOrder_[j];
        dissolveOrder_[j] = temp;
    }
}

bool Transition::update(uint32_t deltaMs) {
    if (!active_) return true;
    
    elapsed_ += deltaMs;
    
    if (elapsed_ >= duration_) {
        active_ = false;
        return true;
    }
    
    return false;
}

void Transition::render(IDisplay* display) {
    if (!active_) return;
    
    float progress = easeInOut(getProgress());
    
    switch (currentType_) {
        case TransitionType::GeometricWipe:
            renderGeometricWipe(display, progress);
            break;
        case TransitionType::Spiral:
            renderSpiral(display, progress);
            break;
        case TransitionType::ConcentricShapes:
            renderConcentricShapes(display, progress);
            break;
        case TransitionType::ScanLines:
            renderScanLines(display, progress);
            break;
        case TransitionType::PixelDissolve:
            renderPixelDissolve(display, progress);
            break;
        case TransitionType::DiamondWipe:
            renderDiamondWipe(display, progress);
            break;
        case TransitionType::None:
        default:
            display->fillScreen(0);
            break;
    }
    
    display->show();
}

void Transition::renderGeometricWipe(IDisplay* display, float progress) {
    display->fillScreen(0);
    
    uint16_t white = display->color565(255, 255, 255);
    
    // Spinning hexagon that expands from center
    int centerX = TRANS_WIDTH / 2;
    int centerY = TRANS_HEIGHT / 2;
    
    // Radius expands from 0 to cover the whole screen
    float maxRadius = sqrtf(centerX * centerX + centerY * centerY);
    float radius = progress * maxRadius * 1.2f;
    
    // Rotation angle (multiple full rotations during transition)
    float angle = progress * 3.14159f * 4;  // 2 full rotations
    
    // Draw hexagon
    int numSides = 6;
    for (int i = 0; i < numSides; i++) {
        float a1 = angle + (i * 2 * 3.14159f / numSides);
        float a2 = angle + ((i + 1) * 2 * 3.14159f / numSides);
        
        int x1 = centerX + (int)(cosf(a1) * radius);
        int y1 = centerY + (int)(sinf(a1) * radius);
        int x2 = centerX + (int)(cosf(a2) * radius);
        int y2 = centerY + (int)(sinf(a2) * radius);
        
        display->drawLine(x1, y1, x2, y2, white);
    }
    
    // Also draw lines from center to vertices for more visual interest
    for (int i = 0; i < numSides; i++) {
        float a = angle + (i * 2 * 3.14159f / numSides);
        int x = centerX + (int)(cosf(a) * radius);
        int y = centerY + (int)(sinf(a) * radius);
        display->drawLine(centerX, centerY, x, y, white);
    }
}

void Transition::renderSpiral(IDisplay* display, float progress) {
    display->fillScreen(0);
    
    uint16_t white = display->color565(255, 255, 255);
    
    int centerX = TRANS_WIDTH / 2;
    int centerY = TRANS_HEIGHT / 2;
    
    // Number of spiral arms
    int numArms = 6;
    
    // Rotation progresses over time
    float baseAngle = progress * 3.14159f * 6;  // 3 full rotations
    
    // Draw each arm
    for (int arm = 0; arm < numArms; arm++) {
        float armAngle = baseAngle + (arm * 2 * 3.14159f / numArms);
        
        // Each arm is a line from center outward
        float maxLen = sqrtf(centerX * centerX + centerY * centerY);
        float len = progress * maxLen * 1.5f;
        
        int x = centerX + (int)(cosf(armAngle) * len);
        int y = centerY + (int)(sinf(armAngle) * len);
        
        display->drawLine(centerX, centerY, x, y, white);
    }
}

void Transition::renderConcentricShapes(IDisplay* display, float progress) {
    display->fillScreen(0);
    
    uint16_t white = display->color565(255, 255, 255);
    
    int centerX = TRANS_WIDTH / 2;
    int centerY = TRANS_HEIGHT / 2;
    
    // Draw multiple expanding squares
    int numRings = 5;
    float maxSize = TRANS_WIDTH > TRANS_HEIGHT ? TRANS_WIDTH : TRANS_HEIGHT;
    
    for (int ring = 0; ring < numRings; ring++) {
        // Stagger the rings - each starts at different progress point
        float ringProgress = progress - (ring * 0.15f);
        if (ringProgress < 0) continue;
        if (ringProgress > 1) ringProgress = 1;
        
        float size = ringProgress * maxSize;
        int halfSize = (int)(size / 2);
        
        // Draw square outline
        int x1 = centerX - halfSize;
        int y1 = centerY - halfSize;
        int x2 = centerX + halfSize;
        int y2 = centerY + halfSize;
        
        // Top
        display->drawFastHLine(x1, y1, x2 - x1, white);
        // Bottom
        display->drawFastHLine(x1, y2, x2 - x1, white);
        // Left
        display->drawFastVLine(x1, y1, y2 - y1, white);
        // Right
        display->drawFastVLine(x2, y1, y2 - y1 + 1, white);
    }
}

void Transition::renderScanLines(IDisplay* display, float progress) {
    display->fillScreen(0);
    
    uint16_t white = display->color565(255, 255, 255);
    
    // Multiple scan lines moving diagonally
    int numLines = 8;
    
    for (int i = 0; i < numLines; i++) {
        // Stagger the lines
        float lineProgress = progress * 2 - (i * 0.1f);
        if (lineProgress < 0) continue;
        if (lineProgress > 1) lineProgress = 1;
        
        // Diagonal line position
        int offset = (int)(lineProgress * (TRANS_WIDTH + TRANS_HEIGHT));
        
        // Draw diagonal line from top-right to bottom-left
        for (int d = 0; d < TRANS_HEIGHT + 10; d++) {
            int x = offset - d;
            int y = d;
            if (x >= 0 && x < TRANS_WIDTH && y >= 0 && y < TRANS_HEIGHT) {
                display->drawPixel(x, y, white);
            }
        }
    }
}

void Transition::renderPixelDissolve(IDisplay* display, float progress) {
    display->fillScreen(0);
    
    uint16_t white = display->color565(255, 255, 255);
    
    // Number of pixels to show based on progress
    int totalPixels = TRANS_WIDTH * TRANS_HEIGHT;
    int pixelsToShow = (int)(progress * totalPixels);
    
    // Draw pixels in shuffled order
    for (int i = 0; i < pixelsToShow && i < totalPixels; i++) {
        int idx = dissolveOrder_[i];
        int x = idx % TRANS_WIDTH;
        int y = idx / TRANS_WIDTH;
        display->drawPixel(x, y, white);
    }
}

void Transition::renderDiamondWipe(IDisplay* display, float progress) {
    display->fillScreen(0);
    
    uint16_t white = display->color565(255, 255, 255);
    
    int centerX = TRANS_WIDTH / 2;
    int centerY = TRANS_HEIGHT / 2;
    
    // Diamond expands from center
    float maxSize = TRANS_WIDTH + TRANS_HEIGHT;  // Manhattan distance to corners
    float size = progress * maxSize;
    
    // Fill pixels inside the diamond (Manhattan distance from center)
    for (int y = 0; y < TRANS_HEIGHT; y++) {
        for (int x = 0; x < TRANS_WIDTH; x++) {
            int dist = abs(x - centerX) + abs(y - centerY);
            if (dist <= (int)size) {
                display->drawPixel(x, y, white);
            }
        }
    }
    
    // Draw diamond outline for extra effect
    int intSize = (int)size;
    // Top vertex to right vertex
    display->drawLine(centerX, centerY - intSize, centerX + intSize, centerY, white);
    // Right to bottom
    display->drawLine(centerX + intSize, centerY, centerX, centerY + intSize, white);
    // Bottom to left
    display->drawLine(centerX, centerY + intSize, centerX - intSize, centerY, white);
    // Left to top
    display->drawLine(centerX - intSize, centerY, centerX, centerY - intSize, white);
}
