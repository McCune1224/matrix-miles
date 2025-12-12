#ifndef PANEL_MANAGER_H
#define PANEL_MANAGER_H

#include "Panel.h"
#include "ButtonManager.h"
#include "Transition.h"

// Maximum number of panels (adjust if needed)
#define MAX_PANELS 8

// PanelManager - manages panel collection and navigation
// Handles button input to switch between panels
// Supports geometric transition effects between panels
// Auto-rotates panels every 5 minutes (configurable)
class PanelManager {
public:
    PanelManager() 
        : panelCount_(0)
        , currentPanel_(0)
        , pendingPanel_(-1)
        , display_(nullptr)
        , transitionEnabled_(true)
        , autoRotateEnabled_(true)
        , autoRotateInterval_(5 * 60 * 1000)  // 5 minutes default
        , timeSinceLastChange_(0) {}
    
    // Set the display to use for rendering
    void setDisplay(IDisplay* display) {
        display_ = display;
    }
    
    // Enable/disable transition effects
    void setTransitionEnabled(bool enabled) {
        transitionEnabled_ = enabled;
    }
    
    // Auto-rotate settings
    void setAutoRotateEnabled(bool enabled) {
        autoRotateEnabled_ = enabled;
    }
    
    void setAutoRotateInterval(uint32_t ms) {
        autoRotateInterval_ = ms;
    }
    
    void resetAutoRotateTimer() {
        timeSinceLastChange_ = 0;
    }
    
    // Add a panel to the manager (returns panel index)
    int addPanel(Panel* panel) {
        if (panelCount_ >= MAX_PANELS) {
            return -1;
        }
        panels_[panelCount_] = panel;
        return panelCount_++;
    }
    
    // Get current panel
    Panel* currentPanel() {
        if (currentPanel_ < panelCount_) {
            return panels_[currentPanel_];
        }
        return nullptr;
    }
    
    // Get panel count
    int panelCount() const { return panelCount_; }
    
    // Get current panel index
    int currentIndex() const { return currentPanel_; }
    
    // Check if transition is playing
    bool isTransitioning() const { return transition_.isActive(); }
    
    // Navigate to next panel (wraps around)
    void nextPanel() {
        if (panelCount_ == 0) return;
        if (transition_.isActive()) return;  // Don't navigate during transition
        
        int nextIdx = (currentPanel_ + 1) % panelCount_;
        startTransition(nextIdx);
    }
    
    // Navigate to previous panel (wraps around)
    void prevPanel() {
        if (panelCount_ == 0) return;
        if (transition_.isActive()) return;  // Don't navigate during transition
        
        int prevIdx = (currentPanel_ - 1 + panelCount_) % panelCount_;
        startTransition(prevIdx);
    }
    
    // Navigate to specific panel by index
    void goToPanel(int index) {
        if (index < 0 || index >= panelCount_) return;
        if (index == currentPanel_) return;
        if (transition_.isActive()) return;  // Don't navigate during transition
        
        startTransition(index);
    }
    
    // Process button event - handles panel navigation
    // Returns true if event was handled
    bool handleButton(ButtonEvent event) {
        if (event == ButtonEvent::None) {
            return false;
        }
        
        // Reset auto-rotate timer on any button press
        resetAutoRotateTimer();
        
        // Ignore buttons during transition
        if (transition_.isActive()) {
            return true;  // Consume the event but don't act
        }
        
        Panel* panel = currentPanel();
        
        // First, let the current panel handle the button
        if (panel) {
            if (event == ButtonEvent::UpPressed && panel->onButtonUp()) {
                return true;
            }
            if (event == ButtonEvent::DownPressed && panel->onButtonDown()) {
                return true;
            }
        }
        
        // If panel didn't handle it, use for navigation
        if (event == ButtonEvent::UpPressed) {
            prevPanel();
            return true;
        } else if (event == ButtonEvent::DownPressed) {
            nextPanel();
            return true;
        }
        
        return false;
    }
    
    // Update current panel and transition (call each frame)
    void update(uint32_t deltaMs) {
        // Update transition if active
        if (transition_.isActive()) {
            bool complete = transition_.update(deltaMs);
            if (complete) {
                // Transition finished - complete the panel switch
                finishTransition();
            }
            return;  // Don't update panel during transition
        }
        
        // Update current panel
        Panel* panel = currentPanel();
        if (panel) {
            panel->update(deltaMs);
        }
        
        // Handle auto-rotate
        if (autoRotateEnabled_ && panelCount_ > 1) {
            timeSinceLastChange_ += deltaMs;
            if (timeSinceLastChange_ >= autoRotateInterval_) {
                timeSinceLastChange_ = 0;
                nextPanel();  // Auto-rotate to next panel
            }
        }
    }
    
    // Render current panel or transition (call each frame)
    void render() {
        if (!display_) return;
        
        // If transition is active, render the transition effect
        if (transition_.isActive()) {
            transition_.render(display_);
            return;
        }
        
        // Otherwise render current panel
        Panel* panel = currentPanel();
        if (panel) {
            panel->render(display_);
        }
    }
    
    // Initialize first panel
    void begin() {
        if (panelCount_ > 0 && panels_[0]) {
            panels_[0]->onEnter();
        }
    }
    
private:
    Panel* panels_[MAX_PANELS];
    int panelCount_;
    int currentPanel_;
    int pendingPanel_;      // Panel to switch to after transition
    IDisplay* display_;
    bool transitionEnabled_;
    Transition transition_;  // Geometric transition effect
    
    // Auto-rotate settings
    bool autoRotateEnabled_;
    uint32_t autoRotateInterval_;   // ms between auto-rotations (default 5 min)
    uint32_t timeSinceLastChange_;  // accumulates deltaMs
    
    // Start transition to a new panel
    void startTransition(int targetPanel) {
        if (!transitionEnabled_) {
            // No transition - switch immediately
            switchPanel(targetPanel);
            return;
        }
        
        // Exit current panel
        Panel* current = panels_[currentPanel_];
        if (current) {
            current->onExit();
        }
        
        // Store pending panel and start transition
        pendingPanel_ = targetPanel;
        timeSinceLastChange_ = 0;  // Reset auto-rotate timer
        transition_.start(TransitionType::Spiral, 400);  // 0.4 second transition
    }
    
    // Complete transition after animation finishes
    void finishTransition() {
        if (pendingPanel_ >= 0) {
            currentPanel_ = pendingPanel_;
            pendingPanel_ = -1;
            
            Panel* next = panels_[currentPanel_];
            if (next) {
                next->onEnter();
            }
        }
    }
    
    // Immediate panel switch (no transition)
    void switchPanel(int targetPanel) {
        Panel* current = panels_[currentPanel_];
        if (current) {
            current->onExit();
        }
        
        currentPanel_ = targetPanel;
        
        Panel* next = panels_[currentPanel_];
        if (next) {
            next->onEnter();
        }
    }
};

#endif // PANEL_MANAGER_H
