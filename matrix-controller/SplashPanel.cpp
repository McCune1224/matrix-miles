#include "SplashPanel.h"

void SplashPanel::update(uint32_t deltaMs) {
    // No animation needed - progress is set externally
}

void SplashPanel::drawProgressBar(IDisplay* display, uint16_t color) {
    // Progress bar dimensions
    // Full width with some padding: X=4 to X=59 (56 pixels wide)
    // Height: 6 pixels, positioned in upper area
    int barX = 4;
    int barY = 6;
    int barWidth = 56;
    int barHeight = 6;
    
    // Draw border (white outline) using lines
    uint16_t white = display->color565(255, 255, 255);
    display->drawFastHLine(barX, barY, barWidth, white);                    // Top
    display->drawFastHLine(barX, barY + barHeight - 1, barWidth, white);    // Bottom
    display->drawFastVLine(barX, barY, barHeight, white);                   // Left
    display->drawFastVLine(barX + barWidth - 1, barY, barHeight, white);    // Right
    
    // Calculate filled width based on progress
    int fillWidth = (barWidth - 2) * progress_ / 100;
    if (fillWidth > 0) {
        display->fillRect(barX + 1, barY + 1, fillWidth, barHeight - 2, color);
    }
}

void SplashPanel::render(IDisplay* display) {
    display->fillScreen(0);
    
    // Use green for the progress bar fill
    uint16_t green = display->color565(0, 255, 0);
    
    // Draw progress bar
    drawProgressBar(display, green);
    
    // Draw status message below the progress bar
    if (statusMessage_ != nullptr) {
        uint16_t white = display->color565(255, 255, 255);
        display->setTextSize(1);
        display->setTextColor(white);
        display->setCursor(4, 16);  // Below the progress bar
        display->print(statusMessage_);
    }
    
    display->show();
}
