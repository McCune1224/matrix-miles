# Matrix Miles

IoT Project to get Strava's running activity tracking system that bridges fitness data with embedded hardware. This project includes a Go web server to handle API / Oauth Logic with Strava that communicates with a MatrixPortal M4 client for displaying data on an LED Matrix (calendar view, monthly / weekly stats, best preformances, etc).

## Overview

Matrix Miles combines a Go backend with a MatrixPortal M4 based client to collect, process, and visualize Strava activity data in an embedded environment.

## Architecture

- **Backend (Go 1.25)**
  - Strava OAuth 2.0 authentication with automatic token refresh
  - PostgreSQL database with comprehensive schema and migrations
  - REST API with API key authentication for IoT devices
  - Structured logging with rotation and an admin log viewer
  - Production deployment on Railway with Docker
- **Embedded Client (CircuitPython for MatrixPortal M4)** ⭐ Recommended
   - Native HTTPS/SSL support (no certificate management headaches)
   - WiFi connectivity with automatic reconnection
   - JSON parsing for activity and calendar data
   - Configurable fetch intervals and user targeting
   - LED matrix display integration with Adafruit ecosystem
   - Live REPL debugging and faster development cycle
- **Legacy C++ Implementation** (esp32_client_cpp/)
   - Arduino-based alternative if CircuitPython needed

## Project Structure

```
matrix-miles/
├── strava-server/              # Go backend (production ready)
│   ├── cmd/main.go             # Application entry point
│   ├── internal/               # Clean architecture layers
│   │   ├── database/           # SQLC generated types and queries
│   │   ├── handlers/           # HTTP request handlers
│   │   ├── services/           # Business logic services
│   │   └── strava/             # Strava API integration
│   ├── db/migrations/          # Database schema evolution
│   └── pkg/                    # Shared utilities
│
├── matrixportal_circuitpython/ # CircuitPython client (RECOMMENDED)
│   ├── code.py                 # Main application
│   ├── config.example.py       # Configuration template
│   ├── lib/                    # Application modules
│   │   ├── api_client.py       # HTTPS API communication
│   │   ├── wifi_manager.py     # WiFi connection handling
│   │   └── calendar_display.py # LED matrix rendering
│   ├── GETTING_STARTED.md      # Setup guide (START HERE!)
│   ├── CIRCUITPYTHON_VS_CPP.md # Why CircuitPython?
│   └── docs/
│       └── API_ENDPOINTS.md    # Backend API reference
│
├── esp32_client_cpp/           # Legacy C++ implementation
│   ├── esp32_client_cpp.ino    # Arduino sketch
│   ├── config.hpp.example      # Configuration template
│   └── lib/                    # C++ modules
│
├── Dockerfile                  # Multi-stage production build
├── docker-compose.yml          # Local development environment
└── railway.json                # Cloud deployment configuration
```

## Current Status

**Completed Components**
- Complete OAuth flow with token management
- Database schema with proper indexing and constraints
- API security with API key authentication
- Structured logging with admin interface
- ✅ **NEW:** CircuitPython client with native HTTPS support
- ✅ **NEW:** Complete setup guides and documentation
- Working MatrixPortal M4 client with JSON parsing
- Docker containerization and Railway deployment

**Next Goals**
- Real-time activity filtering and display customization
- Weekly/monthly statistics display
- Power management and deep sleep optimization
- Multi-user support with device switching

## Technical Highlights
- Security: OAuth 2.0, API keys, and secure token handling
- Observability: Comprehensive logging with admin dashboard
- Embedded Systems: Native HTTPS, WiFi management, and display rendering
- DevOps: Docker, Railway deployment, and development tooling
- CircuitPython Benefits: Simpler HTTPS, faster development, live REPL debugging

## Deployment

**Production**: Automatically deployed on Railway.app
- Backend API: https://matrix-miles-production.up.railway.app
- Health check: /health
- OAuth: /auth/login

## Technology Stack

- Backend: Go 1.25, Echo framework, PostgreSQL, pgx, sqlc, zap logging
- Embedded: CircuitPython 8.x, MatrixPortal M4, Adafruit ecosystem
- Infrastructure: Docker, Railway.app, GitHub Actions
- Alternative: C++/Arduino for embedded (see esp32_client_cpp/)

## Contributing

This is a personal project showcasing an IoT device talking with a dedicated Cloud Server / infra. Feel free to give feedback or if you want help getting this forked / how to set something similar up yourself, reach out :) 

## License

MIT
