#ifndef WEATHER_PANEL_H
#define WEATHER_PANEL_H

#include "Panel.h"

// Weather condition types for icon rendering
enum class WeatherCondition {
    Sunny,
    PartlyCloudy,
    Cloudy,
    Rainy,
    Snowy,
    Windy,
    Stormy,
    Unknown
};

class WeatherPanel : public Panel {
public:
    WeatherPanel();
    
    void onEnter() override;
    void update(uint32_t deltaMs) override;
    void render(IDisplay* display) override;
    const char* name() const override { return "Weather"; }
    
    // Set weather data
    void setWeather(WeatherCondition condition, int tempF, int humidity);
    void setLocation(const char* location);
    
private:
    WeatherCondition condition_;
    int tempF_;
    int humidity_;
    char location_[16];
    uint32_t animTime_;
    
    // Icon drawing helpers (8x8 pixel icons)
    void drawSunIcon(IDisplay* display, int x, int y);
    void drawCloudIcon(IDisplay* display, int x, int y, bool withSun);
    void drawRainIcon(IDisplay* display, int x, int y);
    void drawSnowIcon(IDisplay* display, int x, int y);
    void drawWindIcon(IDisplay* display, int x, int y);
    void drawStormIcon(IDisplay* display, int x, int y);
};

#endif // WEATHER_PANEL_H
