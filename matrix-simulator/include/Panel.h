#ifndef PANEL_H
#define PANEL_H

#include "TerminalDisplay.h"

// Base interface for display panels
// Each panel represents a different view that can be shown on the matrix
class Panel {
public:
    virtual ~Panel() = default;
    
    // Render the panel to the display
    // Called each frame to draw the current state
    virtual void render(IDisplay* display) = 0;
    
    // Update panel state (called each frame for animations)
    // deltaMs is time since last update in milliseconds
    virtual void update(uint32_t deltaMs) = 0;
    
    // Get the panel name (for debugging/display)
    virtual const char* name() const = 0;
    
    // Optional: called when panel becomes active
    virtual void onEnter() {}
    
    // Optional: called when panel becomes inactive
    virtual void onExit() {}
};

#endif // PANEL_H
