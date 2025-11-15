# Matrix Miles

A Strava activity tracking display using an Arduino Nano ESP32 and LED matrix. Helps me with tracking my running activities with a visual calendar display powered by real-time data from the Strava API. Keeps me motivated :)

## Overview
Matrix Miles consists of two main components:

1. **Go Backend Server** - Handles Strava OAuth, token management, and provides a REST API
2. **Arduino Nano ESP32 Client** - C++ client that fetches activity data and displays it on an LED matrix

## Hardware Used

- **Arduino Nano ESP32** (ESP32-S3 based)
- LED Matrix MAX7219 

## Software Used

  - Go 1.25 (for backend server)
  - PostgreSQL 14+ (or Railway/cloud hosted)
  - arduino-cli (for ESP32 development)
  - Neovim (or your preferred editor)
  
- **Strava API:**
  - Strava API credentials ([Get them here](https://www.strava.com/settings/api))

## Project Structure

```
matrix-miles/
├── Dockerfile                  # Root Dockerfile for Railway deployment
├── docker-compose.yml          # Local development with Docker
├── railway.json                # Railway.app configuration
├── .dockerignore              # Docker build exclusions
│
├── strava-server/              # Go backend server
│   ├── cmd/                    # Application entry point
│   ├── internal/               # Internal packages (handlers, database)
│   ├── db/                     # Database migrations and queries
│   ├── RAILWAY_DEPLOYMENT.md   # Railway deployment guide
│   ├── ESP32_PRODUCTION_CONFIG.md # ESP32 production setup
│   └── README.md               # Server documentation
│
├── esp32_client_cpp/           # ESP32 C++ client projects
│   └── blink/                  # ESP32 HTTP client example
│       ├── blink.ino           # Main sketch with API calls
│       └── sketch.yaml         # Arduino CLI config
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

### Docker & Railway Deployment

This monorepo includes production deployment configuration at the root:

- **`Dockerfile`** - Multi-stage build that compiles the strava-server subdirectory
- **`docker-compose.yml`** - Local testing with PostgreSQL
- **`railway.json`** - Railway.app deployment configuration
- **`.dockerignore`** - Excludes ESP32 and other unrelated files from build

**Deploy to Railway:** Push the entire `matrix-miles` repo to GitHub, connect to Railway, and it will automatically build and deploy the strava-server using the root Dockerfile.

## Features

### Backend (WIP)
- Strava OAuth 2.0 authentication
- Automatic token refresh
- Activity syncing from Strava
- Calendar data aggregation
- User statistics
- REST API for ESP32 clients

### Arduino Client (WIP)
- WiFi connectivity
- HTTP client for API calls
- JSON parsing for activity data
- Configuration management
- Future: LED matrix display
- Future: Activity calendar visualization

## Future Enhancements

- [ ] LED matrix display integration (MAX7219)
- [ ] Calendar visualization on matrix
- [ ] Realtime Webhook Updating (as opposed to scheduled / polled updates)
- [ ] Button controls for display modes
- [ ] ?? Deep sleep mode for power saving
- [ ] ?? Multiple user support
- [ ] ?? Web configuration portal

## Contributing

This is a personal project, but suggestions and improvements are welcome!

## License

MIT

## Acknowledgments

- [Strava API](https://developers.strava.com/)
- [Arduino](https://www.arduino.cc/)
- [Echo Framework](https://echo.labstack.com/)
- [arduino-cli](https://github.com/arduino/arduino-cli)
