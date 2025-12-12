#ifndef TRANSITION_H
#define TRANSITION_H

#include "IDisplay.h"

// Display dimensions
#define TRANS_WIDTH 64
#define TRANS_HEIGHT 32

// Available transition animation types
enum class TransitionType {
    None,           // Instant switch, no animation
    GeometricWipe,  // Spinning shape expands from center
    Spiral,         // Pinwheel lines radiate from center
    ConcentricShapes, // Expanding circles/squares from center
    ScanLines,      // Lines sweep across screen
    PixelDissolve,  // Random pixels flip to reveal new panel
    DiamondWipe     // Diamond expands from center
};

// Transition - handles animated transitions between panels
class Transition {
public:
    Transition();
    
    // Start a transition animation
    void start(TransitionType type, uint32_t durationMs = 400);
    
    // Update transition state
    // Returns true when transition is complete
    bool update(uint32_t deltaMs);
    
    // Render the transition frame
    void render(IDisplay* display);
    
    // Check if transition is active
    bool isActive() const { return active_; }
    
    // Stop/reset the transition
    void stop();
    
    // Set the transition type for next transition
    void setType(TransitionType type) { currentType_ = type; }
    TransitionType getType() const { return currentType_; }
    
private:
    bool active_;
    TransitionType currentType_;
    uint32_t elapsed_;
    uint32_t duration_;
    
    // For dissolve effect - track which pixels have been revealed
    uint8_t dissolveOrder_[TRANS_WIDTH * TRANS_HEIGHT];
    int dissolveIndex_;
    
    // Animation progress (0.0 to 1.0)
    float getProgress() const;
    
    // Easing function for smooth animation
    float easeInOut(float t) const;
    
    // Individual animation renderers
    void renderGeometricWipe(IDisplay* display, float progress);
    void renderSpiral(IDisplay* display, float progress);
    void renderConcentricShapes(IDisplay* display, float progress);
    void renderScanLines(IDisplay* display, float progress);
    void renderPixelDissolve(IDisplay* display, float progress);
    void renderDiamondWipe(IDisplay* display, float progress);
    
    // Helper to shuffle dissolve order
    void shuffleDissolveOrder();
};

#endif // TRANSITION_H
