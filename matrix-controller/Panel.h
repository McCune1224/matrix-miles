#ifndef PANEL_H
#define PANEL_H

#include "IDisplay.h"

// Base class for display panels
// Each panel represents a different view (calendar, stats, splash, etc.)
class Panel {
public:
    virtual ~Panel() = default;
    
    // Render the panel to the display
    // Called each frame to draw the current state
    virtual void render(IDisplay* display) = 0;
    
    // Update panel state (called each frame for animations)
    // deltaMs is time since last update in milliseconds
    virtual void update(uint32_t deltaMs) = 0;
    
    // Get the panel name (for debugging)
    virtual const char* name() const = 0;
    
    // Called when panel becomes active (navigated to)
    virtual void onEnter() {}
    
    // Called when panel becomes inactive (navigated away)
    virtual void onExit() {}
    
    // Handle button input - return true if handled
    // Override for panels that need custom button behavior
    virtual bool onButtonUp() { return false; }
    virtual bool onButtonDown() { return false; }
};

#endif // PANEL_H
