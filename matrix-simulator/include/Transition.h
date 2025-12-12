#ifndef TRANSITION_H
#define TRANSITION_H

#include "TerminalDisplay.h"
#include "Panel.h"
#include <functional>

enum class TransitionType {
    None,
    SlideUp,
    SlideDown,
    CrossFade
};

class Transition {
public:
    Transition();
    
    // Start a transition between panels
    void start(TransitionType type, Panel* from, Panel* to, 
               IDisplay* display, uint32_t durationMs = 300);
    
    // Update transition state
    // Returns true when transition is complete
    bool update(uint32_t deltaMs);
    
    // Render the transition frame
    void render(IDisplay* display);
    
    // Check if transition is active
    bool isActive() const { return active_; }
    
    // Get the target panel (for when transition completes)
    Panel* getTargetPanel() const { return toPanel_; }
    
private:
    bool active_;
    TransitionType type_;
    Panel* fromPanel_;
    Panel* toPanel_;
    
    uint32_t elapsed_;
    uint32_t duration_;
    
    // Framebuffers for transition effects
    Color fromBuffer_[MATRIX_WIDTH * MATRIX_HEIGHT];
    Color toBuffer_[MATRIX_WIDTH * MATRIX_HEIGHT];
    
    void capturePanel(Panel* panel, IDisplay* display, Color* buffer);
    float easeInOut(float t);
};

#endif // TRANSITION_H
