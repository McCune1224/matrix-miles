# Matrix Miles

## Overview

IoT Project for fetching Strava API running activity & tracking system that bridges fitness data with embedded hardware. This project includes a Go web server to handle API / Oauth Logic with Strava that communicates with a MatrixPortal M4 microcontroller for displaying data on an LED Matrix (calendar view, monthly / weekly stats, best preformances, etc).

## Architecture

- **Backend (Go 1.25)**
  - Strava OAuth 2.0 authentication with automatic token refresh
  - PostgreSQL database with comprehensive schema and migrations
  - REST API with API key authentication for IoT devices
  - Structured logging with rotation and an admin log viewer
  - Production deployment on Railway with Docker
- **Embedded Client (Good Old C++ and Ardiuno)** 
   - Native HTTP
   - WiFi connectivity with automatic reconnection
   - JSON parsing for activity and calendar data
   - Configurable fetch intervals and user targeting
   - LED matrix display integration with Adafruit ecosystem
   - Live REPL debugging and faster development cycle

## Technical Highlights
- Security: OAuth 2.0, API keys, and secure token handling
- Observability: Comprehensive logging with admin dashboard
- Embedded Systems: Native HTTP, WiFi management, and display rendering
- DevOps: Docker, Railway deployment, and development tooling

## Technology Stack

- Backend: Go 1.25, Echo framework, PostgreSQL, pgx, sqlc, zap logging
- Embedded: CircuitPython 8.x, MatrixPortal M4, Adafruit ecosystem
- Infrastructure: Docker, Railway.app, GitHub Actions
- Alternative: C++/Arduino for embedded (see esp32_client_cpp/)

## Contributing

This is a personal project showcasing an IoT device talking with a dedicated Cloud Server / infra. Feel free to give feedback or if you want help getting this forked / how to set something similar up yourself, reach out :) 

## License

MIT
