#include "WeatherPanel.h"
#include <cstring>
#include <cstdio>

WeatherPanel::WeatherPanel()
    : condition_(WeatherCondition::Sunny)
    , tempF_(72)
    , humidity_(50)
    , animTime_(0) {
    strcpy(location_, "Local");
}

void WeatherPanel::onEnter() {
    animTime_ = 0;
}

void WeatherPanel::update(uint32_t deltaMs) {
    animTime_ += deltaMs;
}

void WeatherPanel::setWeather(WeatherCondition condition, int tempF, int humidity) {
    condition_ = condition;
    tempF_ = tempF;
    humidity_ = humidity;
}

void WeatherPanel::setLocation(const char* location) {
    strncpy(location_, location, sizeof(location_) - 1);
    location_[sizeof(location_) - 1] = '\0';
}

void WeatherPanel::render(IDisplay* display) {
    display->fillScreen(0);
    
    uint16_t white = display->color565(255, 255, 255);
    uint16_t gray = display->color565(100, 100, 100);
    uint16_t cyan = display->color565(0, 200, 255);
    
    // === LEFT SIDE: Weather icon (16x16 area) ===
    int iconX = 4;
    int iconY = 8;
    
    switch (condition_) {
        case WeatherCondition::Sunny:
            drawSunIcon(display, iconX, iconY);
            break;
        case WeatherCondition::PartlyCloudy:
            drawCloudIcon(display, iconX, iconY, true);
            break;
        case WeatherCondition::Cloudy:
            drawCloudIcon(display, iconX, iconY, false);
            break;
        case WeatherCondition::Rainy:
            drawRainIcon(display, iconX, iconY);
            break;
        case WeatherCondition::Snowy:
            drawSnowIcon(display, iconX, iconY);
            break;
        case WeatherCondition::Windy:
            drawWindIcon(display, iconX, iconY);
            break;
        case WeatherCondition::Stormy:
            drawStormIcon(display, iconX, iconY);
            break;
        default:
            // Unknown - draw question mark
            display->setTextColor(gray);
            display->setCursor(iconX + 4, iconY + 4);
            display->print("?");
            break;
    }
    
    // === RIGHT SIDE: Temperature and info ===
    int rightX = 28;
    
    // Temperature - large text
    display->setTextSize(2);
    display->setTextColor(white);
    display->setCursor(rightX, 2);
    char tempStr[8];
    snprintf(tempStr, sizeof(tempStr), "%d", tempF_);
    display->print(tempStr);
    
    // Degree symbol and F (smaller)
    int tempWidth = strlen(tempStr) * 12;  // Size 2 = 12px per char
    display->setTextSize(1);
    display->setTextColor(gray);
    display->setCursor(rightX + tempWidth + 1, 2);
    display->print("F");
    
    // Humidity
    display->setTextColor(cyan);
    display->setCursor(rightX, 20);
    char humStr[8];
    snprintf(humStr, sizeof(humStr), "%d%%", humidity_);
    display->print(humStr);
    
    // Condition text at bottom (optional, small)
    display->setTextColor(gray);
    display->setCursor(2, 26);
    const char* condText = "";
    switch (condition_) {
        case WeatherCondition::Sunny: condText = "SUNNY"; break;
        case WeatherCondition::PartlyCloudy: condText = "P.CLOUDY"; break;
        case WeatherCondition::Cloudy: condText = "CLOUDY"; break;
        case WeatherCondition::Rainy: condText = "RAIN"; break;
        case WeatherCondition::Snowy: condText = "SNOW"; break;
        case WeatherCondition::Windy: condText = "WINDY"; break;
        case WeatherCondition::Stormy: condText = "STORM"; break;
        default: condText = "---"; break;
    }
    display->print(condText);
    
    display->show();
}

// === Icon Drawing Functions (16x16 pixel area) ===

void WeatherPanel::drawSunIcon(IDisplay* display, int x, int y) {
    uint16_t yellow = display->color565(255, 200, 0);
    uint16_t orange = display->color565(255, 150, 0);
    
    // Sun center (6x6 circle approximation)
    display->fillRect(x + 5, y + 3, 6, 2, yellow);
    display->fillRect(x + 4, y + 5, 8, 6, yellow);
    display->fillRect(x + 5, y + 11, 6, 2, yellow);
    
    // Rays - animated pulse
    int pulse = ((animTime_ / 300) % 2);
    uint16_t rayColor = pulse ? orange : yellow;
    
    // Top/bottom rays
    display->drawPixel(x + 7, y + 0, rayColor);
    display->drawPixel(x + 8, y + 0, rayColor);
    display->drawPixel(x + 7, y + 15, rayColor);
    display->drawPixel(x + 8, y + 15, rayColor);
    
    // Left/right rays
    display->drawPixel(x + 0, y + 7, rayColor);
    display->drawPixel(x + 0, y + 8, rayColor);
    display->drawPixel(x + 15, y + 7, rayColor);
    display->drawPixel(x + 15, y + 8, rayColor);
    
    // Diagonal rays
    display->drawPixel(x + 2, y + 2, rayColor);
    display->drawPixel(x + 13, y + 2, rayColor);
    display->drawPixel(x + 2, y + 13, rayColor);
    display->drawPixel(x + 13, y + 13, rayColor);
}

void WeatherPanel::drawCloudIcon(IDisplay* display, int x, int y, bool withSun) {
    uint16_t cloudColor = display->color565(180, 180, 180);
    uint16_t yellow = display->color565(255, 200, 0);
    
    // Small sun peeking out (if partly cloudy)
    if (withSun) {
        display->fillRect(x + 10, y + 2, 4, 4, yellow);
        // Rays
        display->drawPixel(x + 11, y + 0, yellow);
        display->drawPixel(x + 15, y + 3, yellow);
    }
    
    // Cloud shape
    display->fillRect(x + 2, y + 8, 12, 4, cloudColor);
    display->fillRect(x + 4, y + 6, 8, 2, cloudColor);
    display->fillRect(x + 1, y + 10, 2, 2, cloudColor);
    display->fillRect(x + 13, y + 10, 2, 2, cloudColor);
}

void WeatherPanel::drawRainIcon(IDisplay* display, int x, int y) {
    uint16_t cloudColor = display->color565(120, 120, 120);
    uint16_t rainColor = display->color565(0, 150, 255);
    
    // Dark cloud
    display->fillRect(x + 2, y + 4, 12, 4, cloudColor);
    display->fillRect(x + 4, y + 2, 8, 2, cloudColor);
    
    // Rain drops - animated
    int dropOffset = (animTime_ / 150) % 4;
    for (int i = 0; i < 4; i++) {
        int dropY = y + 10 + ((i + dropOffset) % 4);
        int dropX = x + 3 + i * 3;
        display->drawPixel(dropX, dropY, rainColor);
        if (dropY < y + 14) {
            display->drawPixel(dropX, dropY + 1, rainColor);
        }
    }
}

void WeatherPanel::drawSnowIcon(IDisplay* display, int x, int y) {
    uint16_t cloudColor = display->color565(140, 140, 140);
    uint16_t snowColor = display->color565(255, 255, 255);
    
    // Cloud
    display->fillRect(x + 2, y + 2, 12, 4, cloudColor);
    display->fillRect(x + 4, y + 0, 8, 2, cloudColor);
    
    // Snowflakes - animated twinkle
    int phase = (animTime_ / 400) % 3;
    int flakePositions[6][2] = {
        {x + 3, y + 9}, {x + 8, y + 10}, {x + 13, y + 9},
        {x + 5, y + 13}, {x + 10, y + 14}, {x + 6, y + 11}
    };
    
    for (int i = 0; i < 6; i++) {
        if ((i + phase) % 3 != 0) {  // Twinkle effect
            display->drawPixel(flakePositions[i][0], flakePositions[i][1], snowColor);
        }
    }
}

void WeatherPanel::drawWindIcon(IDisplay* display, int x, int y) {
    uint16_t windColor = display->color565(150, 200, 255);
    
    // Wind lines - animated sweep
    int offset = (animTime_ / 100) % 8;
    
    // Three wavy lines
    for (int line = 0; line < 3; line++) {
        int lineY = y + 4 + line * 5;
        int startX = x + (offset + line * 2) % 4;
        
        for (int i = 0; i < 12; i++) {
            int px = startX + i;
            if (px >= x && px < x + 16) {
                display->drawPixel(px, lineY, windColor);
            }
        }
    }
}

void WeatherPanel::drawStormIcon(IDisplay* display, int x, int y) {
    uint16_t cloudColor = display->color565(80, 80, 100);
    uint16_t boltColor = display->color565(255, 255, 0);
    
    // Dark storm cloud
    display->fillRect(x + 1, y + 2, 14, 5, cloudColor);
    display->fillRect(x + 3, y + 0, 10, 2, cloudColor);
    
    // Lightning bolt - animated flash
    int flash = (animTime_ / 200) % 4;
    if (flash < 2) {
        // Bolt shape
        display->drawLine(x + 8, y + 7, x + 6, y + 10, boltColor);
        display->drawLine(x + 6, y + 10, x + 9, y + 10, boltColor);
        display->drawLine(x + 9, y + 10, x + 5, y + 15, boltColor);
    }
}
