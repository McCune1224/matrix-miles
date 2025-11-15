# Matrix Miles

A Strava activity tracking display using an Arduino Nano ESP32 and LED matrix. Track your running activities with a visual calendar display powered by real-time data from the Strava API.

## Overview

Matrix Miles consists of two main components:

1. **Go Backend Server** - Handles Strava OAuth, token management, and provides a REST API
2. **Arduino Nano ESP32 Client** - C++ client that fetches activity data and displays it on an LED matrix

```
┌─────────────────┐
│  Strava API     │
└────────┬────────┘
         │
         ↓
┌─────────────────────────────────┐
│  Go HTTP Server (Echo)          │
│  - OAuth flow handler           │
│  - Token management/refresh     │
│  - PostgreSQL storage           │
│  - REST API for ESP32           │
└────────┬────────────────────────┘
         │
         ↓
┌─────────────────────────────────┐
│  Arduino Nano ESP32             │
│  - C++ HTTP client              │
│  - LED Matrix display           │
│  - WiFi connectivity            │
└─────────────────────────────────┘
```

## Hardware Requirements

- **Arduino Nano ESP32** (ESP32-S3 based)
- USB-C cable
- LED Matrix (MAX7219 or similar) - optional for initial testing
- WiFi network

## Software Requirements

- **Development Machine:**
  - Go 1.21+ (for backend server)
  - PostgreSQL 14+ (or Railway/cloud hosted)
  - arduino-cli (for ESP32 development)
  - Neovim (or your preferred editor)
  
- **Strava API:**
  - Strava API credentials ([Get them here](https://www.strava.com/settings/api))

## Quick Start

### 1. Backend Server Setup

The Go server is already configured and working. See [strava-server/README.md](strava-server/README.md) for details.

```bash
cd strava-server
cp .env.example .env
# Edit .env with your credentials
go run ./cmd/main.go
```

Visit `http://localhost:8080/auth/login` to complete OAuth flow.

### 2. Arduino Nano ESP32 Setup

See [ARDUINO_NANO_ESP32_SETUP.md](ARDUINO_NANO_ESP32_SETUP.md) for complete setup instructions.

```bash
# Install arduino-cli
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

# Install ESP32 core
arduino-cli core install arduino:esp32

# Install required libraries
arduino-cli lib install ArduinoJson
```

### 3. C++ Client Development

See [CPP_CLIENT_GUIDE.md](CPP_CLIENT_GUIDE.md) for detailed C++ client development guide.

### 4. Neovim Setup

See [NEOVIM_SETUP.md](NEOVIM_SETUP.md) for configuring Neovim for Arduino C++ development.

## Project Structure

```
matrix-miles/
├── strava-server/              # Go backend server
│   ├── cmd/                    # Application entry point
│   ├── internal/               # Internal packages (handlers, database)
│   ├── db/                     # Database migrations and queries
│   └── README.md               # Server documentation
│
├── arduino_client/             # Arduino Nano ESP32 C++ client
│   ├── arduino_client.ino      # Main sketch
│   ├── config.h                # Configuration
│   ├── api_client.cpp/h        # HTTP API client
│   └── display.cpp/h           # LED matrix control
│
├── c-proof-of-concept/         # C prototype (reference only)
│
├── archive/                    # Old MicroPython documentation
│
├── README.md                   # This file
├── ARDUINO_NANO_ESP32_SETUP.md # Hardware setup guide
├── CPP_CLIENT_GUIDE.md         # C++ development guide
├── NEOVIM_SETUP.md             # Editor setup guide
├── NEXT_SESSION.md             # Quick reference for next work session
└── SETUP_COMPLETE.md           # Current project status
```

## Documentation

- **[ARDUINO_NANO_ESP32_SETUP.md](ARDUINO_NANO_ESP32_SETUP.md)** - Complete Arduino Nano ESP32 setup with arduino-cli
- **[CPP_CLIENT_GUIDE.md](CPP_CLIENT_GUIDE.md)** - C++ client development guide
- **[NEOVIM_SETUP.md](NEOVIM_SETUP.md)** - Neovim configuration for C++ development
- **[NEXT_SESSION.md](NEXT_SESSION.md)** - Quick start guide for your next work session
- **[SETUP_COMPLETE.md](SETUP_COMPLETE.md)** - Current project status and credentials
- **[strava-server/README.md](strava-server/README.md)** - Backend server documentation

## Current Status

✅ **Go Backend Server** - Complete and functional
- OAuth flow working
- Token management implemented
- REST API endpoints ready
- PostgreSQL database configured
- Deployed on Railway

🔨 **Arduino Nano ESP32 Client** - Ready for development
- Hardware acquired
- Development environment documented
- C++ client guide ready
- Neovim setup guide prepared

⏳ **LED Matrix Display** - Future enhancement
- Will be added after basic HTTP client working

## Features

### Backend (Complete)
- Strava OAuth 2.0 authentication
- Automatic token refresh
- Activity syncing from Strava
- Calendar data aggregation
- User statistics
- REST API for ESP32 clients

### Arduino Client (In Development)
- WiFi connectivity
- HTTP client for API calls
- JSON parsing for activity data
- Configuration management
- Future: LED matrix display
- Future: Activity calendar visualization

## API Endpoints

The backend provides the following endpoints for the ESP32 client:

- `GET /health` - Health check
- `GET /api/activities/recent/:userId` - Fetch recent activities
- `GET /api/activities/calendar/:userId/:year/:month` - Calendar data
- `GET /api/stats/:userId` - User statistics

All API endpoints require `X-API-Key` header for authentication.

## Development Workflow

### Typical Session

1. **Start backend server:**
   ```bash
   cd strava-server
   air  # or: go run ./cmd/main.go
   ```

2. **Develop Arduino client:**
   ```bash
   cd arduino_client
   nvim arduino_client.ino
   
   # Compile
   arduino-cli compile --fqbn arduino:esp32:nano_esp32 .
   
   # Upload
   arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:esp32:nano_esp32 .
   
   # Monitor
   arduino-cli monitor -p /dev/ttyACM0
   ```

3. **Test API locally:**
   ```bash
   # Get activities
   curl -H "X-API-Key: your_api_key" \
     http://localhost:8080/api/activities/recent/1
   ```

## Troubleshooting

### Backend Issues
See [strava-server/README.md](strava-server/README.md#troubleshooting)

### Arduino Issues
See [ARDUINO_NANO_ESP32_SETUP.md](ARDUINO_NANO_ESP32_SETUP.md#troubleshooting)

### Common Problems

**WiFi not connecting:**
- Check SSID and password in config.h
- Verify 2.4GHz network (ESP32 doesn't support 5GHz)

**Can't reach server:**
- Use your machine's local IP, not localhost
- Check firewall settings
- Verify server is running

**API authentication failed:**
- Verify API key matches in .env and config.h
- Check X-API-Key header is being sent

## Future Enhancements

- [ ] LED matrix display integration (MAX7219)
- [ ] Calendar visualization on matrix
- [ ] Activity type indicators (run/ride/swim)
- [ ] Button controls for display modes
- [ ] Deep sleep mode for power saving
- [ ] OTA (Over-The-Air) updates
- [ ] Multiple user support
- [ ] Web configuration portal

## Contributing

This is a personal project, but suggestions and improvements are welcome!

## License

MIT

## Acknowledgments

- [Strava API](https://developers.strava.com/)
- [Arduino](https://www.arduino.cc/)
- [Echo Framework](https://echo.labstack.com/)
- [arduino-cli](https://github.com/arduino/arduino-cli)

## Support

For detailed setup instructions, see the documentation files listed above. For server-specific issues, check [strava-server/README.md](strava-server/README.md).
