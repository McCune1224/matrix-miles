# Matrix Miles

A production-ready Strava activity tracking system that bridges fitness data with embedded hardware. This project includes a Go backend API and an ESP32 client for real world IoT deployments.

## Overview

Matrix Miles combines a Go backend with an ESP32 based client to collect, process, and visualize Strava activity data in an embedded environment.

## Architecture

- **Backend (Go 1.25)**
  - Strava OAuth 2.0 authentication with automatic token refresh
  - PostgreSQL database with comprehensive schema and migrations
  - REST API with API key authentication for IoT devices
  - Structured logging with rotation and an admin log viewer
  - Production deployment on Railway with Docker
- **Embedded Client (C/C++ for ESP32)**
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

**Production Ready Components**
- Complete OAuth flow with token management
- Database schema with proper indexing and constraints
- API security with API key authentication
- Structured logging with admin interface
- Working ESP32 HTTP client with JSON parsing
- Docker containerization and Railway deployment

**Next Phase**
- LED matrix display integration (MAX7219)
- Calendar visualization algorithms
- Power management and deep sleep optimization

## Technical Highlights

- Clean Architecture: Well-structured Go backend with separation of concerns
- Database Design: Proper schema with relationships, indexes, and migrations
- Security: OAuth 2.0, API keys, and secure token handling
- Observability: Comprehensive logging with admin dashboard
- Embedded Systems: Robust WiFi and HTTP client implementation
- DevOps: Docker, Railway deployment, and development tooling

## Deployment

**Production**: Automatically deployed on Railway.app
- Backend API: https://matrix-miles-production.up.railway.app
- Health check: /health
- OAuth: /auth/login

## API Endpoints

**Public (OAuth)**
- GET /auth/login - Initiate Strava OAuth flow
- GET /auth/callback - Handle OAuth callback

**Protected (API Key Required)**
- GET /api/activities/recent/:userId - Recent activities
- GET /api/activities/calendar/:userId/:year/:month - Monthly calendar data
- GET /api/stats/:userId - User statistics

**Admin (Basic Auth)**
- POST /admin/sync/:userId - Force activity sync
- GET /admin/logs - View application logs
- GET /admin/logs/level/:level - Filter logs by level

## Hardware Requirements

- ESP32 development board (tested with ESP32-S3)
- Optional: MAX7219 LED matrix for display (future phase)

## Technology Stack

- Backend: Go 1.25, Echo framework, PostgreSQL, pgx, sqlc, zap logging
- Embedded: Arduino C/C++, ESP32 WiFi, HTTPClient, ArduinoJson
- Infrastructure: Docker, Railway.app, GitHub Actions

## Contributing

This is a personal project showcasing full-stack IoT development. The codebase demonstrates production-ready practices for embedded systems integration with modern backend services.

## License

MIT
