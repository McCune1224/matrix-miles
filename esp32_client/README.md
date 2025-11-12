# ESP32 MicroPython Client for Strava Server

This directory contains the ESP32 MicroPython code for fetching and displaying Strava activity data from your Go server.

## Files

- **config.py** - Configuration file with WiFi credentials, server URL, API key, and user ID
- **boot.py** - Runs automatically on ESP32 startup, connects to WiFi
- **api_client.py** - HTTP client for communicating with the Go server
- **main.py** - Main application logic that fetches and displays activity data

## Quick Start

### 1. Update Configuration

Edit `config.py` and update:
- `WIFI_SSID` - Your WiFi network name
- `WIFI_PASSWORD` - Your WiFi password
- `API_BASE_URL` - Your server URL (e.g., `http://192.168.1.100:8080`)
- `USER_ID` - Your user ID from the database

### 2. Flash MicroPython Firmware

```bash
# Download firmware
wget https://micropython.org/resources/firmware/ESP32_GENERIC-20240602-v1.23.0.bin

# Erase flash
esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash

# Flash MicroPython
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x1000 ESP32_GENERIC-20240602-v1.23.0.bin
```

### 3. Upload Files to ESP32

```bash
# Install urequests library
mpremote connect /dev/ttyUSB0 mip install urequests

# Upload files
mpremote connect /dev/ttyUSB0 fs cp config.py :config.py
mpremote connect /dev/ttyUSB0 fs cp boot.py :boot.py
mpremote connect /dev/ttyUSB0 fs cp api_client.py :api_client.py
mpremote connect /dev/ttyUSB0 fs cp main.py :main.py
```

### 4. Run

```bash
# Connect to REPL
mpremote connect /dev/ttyUSB0

# Run main
>>> import main
```

## Requirements

- ESP32 board with WiFi
- MicroPython firmware
- Python 3.7+ with esptool and mpremote
- Running Go server (strava-server)
- WiFi network

## Troubleshooting

See the main **ESP32_SETUP_GUIDE.md** in the parent directory for detailed troubleshooting steps.

## Next Steps

- Add LED matrix display
- Implement deep sleep for battery saving
- Add button controls for different display modes
- Show calendar view on matrix

## API Endpoints Used

- `GET /health` - Health check
- `GET /api/activities/recent/:userId` - Recent activities
- `GET /api/stats/:userId` - User statistics
- `GET /api/activities/calendar/:userId/:year/:month` - Calendar data
