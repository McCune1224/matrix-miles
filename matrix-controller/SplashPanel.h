#ifndef SPLASH_PANEL_H
#define SPLASH_PANEL_H

#include "Panel.h"

// SplashPanel - loading screen with progress bar and status text
// Shows progress bar that fills as loading steps complete
// Displays status message indicating current loading step
class SplashPanel : public Panel {
public:
    SplashPanel() : progress_(0), statusMessage_(nullptr) {}
    
    void render(IDisplay* display) override;
    void update(uint32_t deltaMs) override;
    const char* name() const override { return "Splash"; }
    
    void onEnter() override {
        progress_ = 0;
        statusMessage_ = nullptr;
    }
    
    // Set progress (0-100) and status message
    void setProgress(int percent, const char* msg) {
        progress_ = percent;
        statusMessage_ = msg;
    }
    
    // Clear status (loading complete)
    void clearStatus() {
        statusMessage_ = nullptr;
        progress_ = 100;
    }
    
private:
    int progress_;  // 0-100 percent
    const char* statusMessage_;  // Current status text
    
    // Draw the progress bar
    void drawProgressBar(IDisplay* display, uint16_t color);
};

#endif // SPLASH_PANEL_H
