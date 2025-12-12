# Agent Guidelines for Matrix Miles

## User Background
User primarily does backend web server development; C++, embedded systems, and breadboard programming are new to them.

## Project Overview
Dual-platform codebase: Go backend (strava-server/) + MatrixPortal M4 C++ client (esp32_client_cpp/)

## Build & Test Commands

### Go Backend (strava-server/)
- Build: `cd strava-server && make build` or `go build -o server ./cmd/main.go`
- Run: `make run` or `air` (hot reload)
- Test all: `go test -v ./...`
- Test single package: `go test -v ./internal/handlers`
- Lint: `make lint` (requires golangci-lint)
- Format: `make fmt` (go fmt + gofmt -s)
- Pre-commit: `make prep` (fmt, vet, generate, lint, test)
- Generate DB code: `sqlc generate` (after modifying db/queries/*.sql)

### MatrixPortal M4 Client (esp32_client_cpp/)
- Compile: `cd esp32_client_cpp && make compile` (arduino-cli)
- Upload: `make upload` (requires PORT=/dev/ttyACM0)
- Monitor: `make monitor`

## Code Style Guidelines

### Go Backend
- **Imports**: Standard lib → third-party → internal (grouped, alphabetical)
- **Types**: Always explicit, use pgtype for nullable DB fields, context.Context first param
- **Naming**: camelCase locals, PascalCase exports, descriptive names (getUserActivities not getUA)
- **Error handling**: Return errors up, wrap with context using fmt.Errorf("context: %w", err)
- **Logging**: Use structured zap logger: `logger.Info("msg", zap.String("key", val))`
- **DB queries**: Use SQLC generated code, context timeouts for long queries
- **Handlers**: Return echo.NewHTTPError with proper status codes
- **Comments**: Godoc style for exports, inline for complex logic

### MatrixPortal M4 C++
- **Includes**: Arduino core → ESP32 libs → third-party → local headers
- **Naming**: camelCase methods, UPPER_CASE constants, descriptive (connectWiFi not cw)
- **Memory**: Use const char* for strings, avoid String objects, be mindful of heap
- **Error handling**: Return bool success, Serial.println for debug output
- **Configuration**: Use config.hpp for credentials (never commit)
- **Comments**: Brief function descriptions, explain hardware-specific logic

## Workflow Preferences
- **Never make commits** - User handles all git commits manually
- **Local server URL**: `http://192.168.68.100:8080` (Strava server on local network)
