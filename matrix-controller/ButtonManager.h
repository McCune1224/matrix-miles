#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>

// MatrixPortal M4 button pins
// Note: Buttons pull LOW when pressed (no internal pull-ups)
#define BUTTON_UP_PIN   2
#define BUTTON_DOWN_PIN 3

// Button event types
enum class ButtonEvent {
    None,
    UpPressed,
    DownPressed
};

// ButtonManager - handles button debouncing and event detection
// Buttons on MatrixPortal M4 have no pull-up resistors, so we use INPUT_PULLUP
class ButtonManager {
public:
    ButtonManager() 
        : lastUpState_(HIGH)
        , lastDownState_(HIGH)
        , lastDebounceTimeUp_(0)
        , lastDebounceTimeDown_(0)
        , upPressed_(false)
        , downPressed_(false) {}
    
    // Initialize button pins - call in setup()
    void begin() {
        pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
        pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
    }
    
    // Poll buttons and return any new event
    // Call this every frame in loop()
    ButtonEvent update() {
        unsigned long currentTime = millis();
        ButtonEvent event = ButtonEvent::None;
        
        // Read current button states (LOW = pressed)
        int upReading = digitalRead(BUTTON_UP_PIN);
        int downReading = digitalRead(BUTTON_DOWN_PIN);
        
        // Debounce UP button
        if (upReading != lastUpState_) {
            lastDebounceTimeUp_ = currentTime;
        }
        
        if ((currentTime - lastDebounceTimeUp_) > DEBOUNCE_DELAY_MS) {
            // State has been stable long enough
            if (upReading == LOW && !upPressed_) {
                // Button just pressed
                upPressed_ = true;
                event = ButtonEvent::UpPressed;
            } else if (upReading == HIGH) {
                upPressed_ = false;
            }
        }
        
        lastUpState_ = upReading;
        
        // Debounce DOWN button (only if UP didn't trigger)
        if (event == ButtonEvent::None) {
            if (downReading != lastDownState_) {
                lastDebounceTimeDown_ = currentTime;
            }
            
            if ((currentTime - lastDebounceTimeDown_) > DEBOUNCE_DELAY_MS) {
                if (downReading == LOW && !downPressed_) {
                    downPressed_ = true;
                    event = ButtonEvent::DownPressed;
                } else if (downReading == HIGH) {
                    downPressed_ = false;
                }
            }
        }
        
        lastDownState_ = downReading;
        
        return event;
    }
    
    // Check if UP button is currently held
    bool isUpHeld() const { return upPressed_; }
    
    // Check if DOWN button is currently held
    bool isDownHeld() const { return downPressed_; }
    
private:
    static constexpr unsigned long DEBOUNCE_DELAY_MS = 50;
    
    int lastUpState_;
    int lastDownState_;
    unsigned long lastDebounceTimeUp_;
    unsigned long lastDebounceTimeDown_;
    bool upPressed_;
    bool downPressed_;
};

#endif // BUTTON_MANAGER_H
