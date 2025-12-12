package handlers

import (
	"encoding/json"
	"fmt"
	"net/http"
	"sync"
	"time"

	"github.com/labstack/echo/v4"
)

// WeatherHandler handles weather-related API requests
// Uses Open-Meteo API (free, no API key required)
type WeatherHandler struct {
	lat    string
	lon    string
	client *http.Client

	// Cached location from IP geolocation
	cachedLat  string
	cachedLon  string
	locMu      sync.RWMutex
	locFetched bool
}

// NewWeatherHandler creates a new weather handler
func NewWeatherHandler(lat, lon string) *WeatherHandler {
	return &WeatherHandler{
		lat:    lat,
		lon:    lon,
		client: &http.Client{Timeout: 10 * time.Second},
	}
}

// WeatherResponse is the simplified response for the ESP32
type WeatherResponse struct {
	Condition   string  `json:"condition"`   // sunny, cloudy, rainy, snowy, windy, stormy, partly_cloudy
	TempF       int     `json:"temp_f"`      // Temperature in Fahrenheit
	Humidity    int     `json:"humidity"`    // Humidity percentage
	Description string  `json:"description"` // Human-readable description
	WindSpeed   float64 `json:"wind_speed"`  // Wind speed in mph
}

// OpenMeteoResponse is the response from Open-Meteo API
type OpenMeteoResponse struct {
	Current struct {
		Temperature   float64 `json:"temperature_2m"`
		Humidity      int     `json:"relative_humidity_2m"`
		WeatherCode   int     `json:"weather_code"`
		WindSpeed     float64 `json:"wind_speed_10m"`
		Precipitation float64 `json:"precipitation"`
	} `json:"current"`
}

// IPAPIResponse is the response from ip-api.com (free, no key needed)
type IPAPIResponse struct {
	Lat float64 `json:"lat"`
	Lon float64 `json:"lon"`
}

// getLocation returns lat/lon, auto-detecting from IP if not configured
func (h *WeatherHandler) getLocation() (lat, lon string, err error) {
	// If explicitly configured, use those values
	if h.lat != "" && h.lon != "" {
		return h.lat, h.lon, nil
	}

	// Check cache first
	h.locMu.RLock()
	if h.locFetched {
		lat, lon = h.cachedLat, h.cachedLon
		h.locMu.RUnlock()
		return lat, lon, nil
	}
	h.locMu.RUnlock()

	// Fetch location from IP geolocation API (free, no key needed)
	// Using ip-api.com - 45 requests/minute limit (plenty for our use)
	resp, err := h.client.Get("http://ip-api.com/json/?fields=lat,lon")
	if err != nil {
		return "", "", fmt.Errorf("failed to fetch location: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return "", "", fmt.Errorf("IP geolocation API returned status %d", resp.StatusCode)
	}

	var ipResp IPAPIResponse
	if err := json.NewDecoder(resp.Body).Decode(&ipResp); err != nil {
		return "", "", fmt.Errorf("failed to parse location: %w", err)
	}

	// Cache the result
	h.locMu.Lock()
	h.cachedLat = fmt.Sprintf("%.4f", ipResp.Lat)
	h.cachedLon = fmt.Sprintf("%.4f", ipResp.Lon)
	h.locFetched = true
	lat, lon = h.cachedLat, h.cachedLon
	h.locMu.Unlock()

	return lat, lon, nil
}

// GetCurrentWeather returns the current weather for the configured location
func (h *WeatherHandler) GetCurrentWeather(c echo.Context) error {
	lat, lon, err := h.getLocation()
	if err != nil {
		return echo.NewHTTPError(http.StatusInternalServerError, "Failed to determine location: "+err.Error())
	}

	// Build Open-Meteo API URL (free, no API key needed)
	url := fmt.Sprintf(
		"https://api.open-meteo.com/v1/forecast?latitude=%s&longitude=%s&current=temperature_2m,relative_humidity_2m,precipitation,weather_code,wind_speed_10m&temperature_unit=fahrenheit&wind_speed_unit=mph",
		lat, lon,
	)

	resp, err := h.client.Get(url)
	if err != nil {
		return echo.NewHTTPError(http.StatusInternalServerError, "Failed to fetch weather: "+err.Error())
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return echo.NewHTTPError(http.StatusInternalServerError, "Weather API returned error")
	}

	var meteoResp OpenMeteoResponse
	if err := json.NewDecoder(resp.Body).Decode(&meteoResp); err != nil {
		return echo.NewHTTPError(http.StatusInternalServerError, "Failed to parse weather: "+err.Error())
	}

	// Map Open-Meteo weather code to our simplified conditions
	condition, description := mapWeatherCode(meteoResp.Current.WeatherCode, meteoResp.Current.WindSpeed)

	weather := WeatherResponse{
		Condition:   condition,
		TempF:       int(meteoResp.Current.Temperature),
		Humidity:    meteoResp.Current.Humidity,
		WindSpeed:   meteoResp.Current.WindSpeed,
		Description: description,
	}

	return c.JSON(http.StatusOK, weather)
}

// mapWeatherCode maps Open-Meteo WMO weather codes to our simplified conditions
// See: https://open-meteo.com/en/docs (WMO Weather interpretation codes)
func mapWeatherCode(code int, windSpeed float64) (condition, description string) {
	// Check for high wind first (> 20 mph)
	if windSpeed > 20 {
		return "windy", "Windy"
	}

	switch code {
	case 0:
		return "sunny", "Clear sky"
	case 1:
		return "sunny", "Mainly clear"
	case 2:
		return "partly_cloudy", "Partly cloudy"
	case 3:
		return "cloudy", "Overcast"
	case 45, 48:
		return "cloudy", "Foggy"
	case 51, 53, 55:
		return "rainy", "Drizzle"
	case 56, 57:
		return "rainy", "Freezing drizzle"
	case 61, 63, 65:
		return "rainy", "Rain"
	case 66, 67:
		return "rainy", "Freezing rain"
	case 71, 73, 75:
		return "snowy", "Snow"
	case 77:
		return "snowy", "Snow grains"
	case 80, 81, 82:
		return "rainy", "Rain showers"
	case 85, 86:
		return "snowy", "Snow showers"
	case 95:
		return "stormy", "Thunderstorm"
	case 96, 99:
		return "stormy", "Thunderstorm with hail"
	default:
		return "unknown", "Unknown"
	}
}
