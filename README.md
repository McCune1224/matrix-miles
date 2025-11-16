# Matrix Miles

IoT Project to get Strava's running activity tracking system that bridges fitness data with embedded hardware. This project includes a Go web server to handle API / Oauth Logic with Strava that communicates with an ESP32 client for displaying data on an LED Matrix (calendar view, monthly / weekly stats, best preformances, etc).

## Overview

Matrix Miles combines a Go backend with an ESP32 based client to collect, process, and visualize Strava activity data in an embedded environment.

## Architecture

- **Backend (Go 1.25)**
  - Strava OAuth 2.0 authentication with automatic token refresh
  - PostgreSQL database with comprehensive schema and migrations
  - REST API with API key authentication for IoT devices
  - Structured logging with rotation and an admin log viewer
  - Production deployment on Railway with Docker
- **Embedded Client (C++ for ESP32)**
  - WiFi connectivity with automatic reconnection
  - HTTP client for secure API communication
  - JSON parsing for activity and calendar data
  - Configurable fetch intervals and user targeting
  - Ready for LED matrix display integration

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
├── esp32_client_cpp/           # ESP32 client (functional)
│   ├── blink.ino               # Working HTTP client implementation
│   ├── config.h.example        # Configuration template
│   └── Makefile                # Build automation
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
- Working ESP32 HTTP client with JSON parsing
- Docker containerization and Railway deployment

**Next Goals**
- LED matrix display integration (MAX7219)
- Calendar visualization algorithms
- Power management and deep sleep optimization

## Technical Highlights
- Security: OAuth 2.0, API keys, and secure token handling
- Observability: Comprehensive logging with admin dashboard
- Embedded Systems: Robust WiFi and HTTP client implementation
- DevOps: Docker, Railway deployment, and development tooling

## Deployment

**Production**: Automatically deployed on Railway.app
- Backend API: https://matrix-miles-production.up.railway.app
- Health check: /health
- OAuth: /auth/login

## Technology Stack

- Backend: Go 1.25, Echo framework, PostgreSQL, pgx, sqlc, zap logging
- Embedded: Arduino C/C++, ESP32 WiFi, HTTPClient, ArduinoJson
- Infrastructure: Docker, Railway.app, GitHub Actions

## Contributing

This is a personal project showcasing an IoT device talking with a dedicated Cloud Server / infra. Feel free to give feedback or if you want help getting this forked / how to set something similar up yourself, reach out :) 

## License

MIT
