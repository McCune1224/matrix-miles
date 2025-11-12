# Strava OAuth Server

A Go HTTP server using Echo framework that handles Strava OAuth authentication, manages user tokens, and provides a REST API for ESP32 devices to fetch activity data.

## Features

- ✅ Strava OAuth 2.0 authentication flow
- ✅ Automatic token refresh
- ✅ PostgreSQL database with sqlc-generated queries
- ✅ REST API for ESP32 clients
- ✅ Activity syncing from Strava API
- ✅ Calendar data aggregation
- ✅ User statistics

## Architecture

```
┌─────────────┐
│   Browser   │ → OAuth flow
└──────┬──────┘
       ↓
┌──────────────────┐
│  Go HTTP Server  │
│  (Echo + sqlc)   │
└────┬─────────┬───┘
     ↓         ↓
┌────────┐  ┌──────┐
│ Postgres│  │ESP32 │
└─────────┘  └──────┘
```

## Tech Stack

- **Web Framework**: [Echo](https://echo.labstack.com/)
- **Database**: PostgreSQL
- **Database Queries**: [sqlc](https://sqlc.dev/) + [sqlx](https://github.com/jmoiron/sqlx)
- **OAuth Client**: Custom Strava implementation
- **Config**: [godotenv](https://github.com/joho/godotenv)

## Prerequisites

- Go 1.21+
- PostgreSQL 14+
- Strava API credentials ([Get them here](https://www.strava.com/settings/api))
- sqlc installed (`go install github.com/sqlc-dev/sqlc/cmd/sqlc@latest`)

## Quick Start

### 1. Clone and Setup

```bash
cd strava-server

# Copy example env file
cp .env.example .env

# Edit .env with your credentials
nano .env
```

### 2. Configure Environment Variables

Edit `.env` with your values:

```env
# Server
PORT=8080
DOMAIN=http://localhost:8080

# Strava API (get from https://www.strava.com/settings/api)
STRAVA_CLIENT_ID=your_client_id
STRAVA_CLIENT_SECRET=your_client_secret
STRAVA_REDIRECT_URI=http://localhost:8080/auth/callback

# Database
DB_HOST=localhost
DB_PORT=5432
DB_USER=strava_user
DB_PASSWORD=your_secure_password
DB_NAME=strava_db
DB_SSLMODE=disable

# Security
ESP32_API_KEY=generate_secure_random_key_here
ADMIN_USERNAME=admin
ADMIN_PASSWORD=change_this_password
```

**Generate a secure API key:**
```bash
openssl rand -hex 32
```

### 3. Setup Database

```bash
# Create database
createdb strava_db

# Or using psql
psql -U postgres -c "CREATE DATABASE strava_db;"

# Run migrations
psql -U strava_user -d strava_db -f db/migrations/001_initial_schema.sql
```

### 4. Install Dependencies

```bash
go mod download
```

### 5. Build and Run

#### Production Build
```bash
# Build
go build -o server ./cmd/server

# Run
./server
```

#### Development with Hot Reload
```bash
# Install Air (if not already installed)
go install github.com/air-verse/air@latest

# Run with hot reload
air
```

Air will automatically rebuild and restart the server when you make changes to `.go` files.

Or run directly without building:
```bash
go run ./cmd/server/main.go
```

You should see:
```
✓ Database connected successfully
🚀 Server starting on :8080
📍 OAuth login: http://localhost:8080/auth/login
```

## API Endpoints

### OAuth Endpoints (Public)

#### Initiate OAuth Flow
```
GET /auth/login
```
Redirects to Strava authorization page.

#### OAuth Callback
```
GET /auth/callback?code={auth_code}
```
Handles OAuth callback and stores user tokens.

### ESP32 API Endpoints (Requires API Key)

All endpoints require `X-API-Key` header.

#### Get Recent Activities
```bash
curl -H "X-API-Key: your_api_key" \
  http://localhost:8080/api/activities/recent/1
```

Response:
```json
[
  {
    "id": 1,
    "user_id": 1,
    "strava_activity_id": 123456789,
    "name": "Morning Run",
    "type": "Run",
    "distance": 5000.0,
    "moving_time": 1800,
    "start_date": "2025-11-10T08:00:00Z"
  }
]
```

#### Get Calendar Data
```bash
curl -H "X-API-Key: your_api_key" \
  http://localhost:8080/api/activities/calendar/1/2025/11
```

Response:
```json
[
  {
    "activity_date": "2025-11-10",
    "count": 2,
    "total_distance": 10000.0
  }
]
```

#### Get User Stats
```bash
curl -H "X-API-Key: your_api_key" \
  http://localhost:8080/api/stats/1
```

Response:
```json
{
  "total_activities": 42,
  "total_distance": 250000.0,
  "total_time": 86400
}
```

### Admin Endpoints (Requires Basic Auth)

#### Sync Activities
```bash
curl -u admin:password \
  -X POST http://localhost:8080/admin/sync/1
```

Response:
```json
{
  "message": "Sync completed",
  "fetched": 30,
  "saved": 25
}
```

## Project Structure

```
strava-server/
├── cmd/
│   └── server/
│       └── main.go              # Application entry point
├── internal/
│   ├── database/                # sqlc generated code
│   │   ├── activities.sql.go
│   │   ├── users.sql.go
│   │   ├── models.go
│   │   └── querier.go
│   ├── handlers/                # HTTP handlers
│   │   ├── oauth.go            # OAuth flow handlers
│   │   └── api.go              # API handlers
│   └── strava/                 # Strava API client
│       └── client.go
├── pkg/
│   └── config/                 # Configuration management
│       └── config.go
├── db/
│   ├── migrations/             # Database schemas
│   │   └── 001_initial_schema.sql
│   └── queries/                # SQL queries for sqlc
│       ├── users.sql
│       └── activities.sql
├── sqlc.yaml                   # sqlc configuration
├── go.mod
├── go.sum
├── .env.example
├── .gitignore
└── README.md
```

## Development

### Regenerate sqlc Code

After modifying SQL queries:

```bash
sqlc generate
```

### Database Migrations

To add a new migration:

```bash
# Create new migration file
touch db/migrations/002_add_new_table.sql

# Apply migration
psql -U strava_user -d strava_db -f db/migrations/002_add_new_table.sql
```

### Testing OAuth Flow

1. Start the server
2. Visit http://localhost:8080/auth/login
3. Authorize with Strava
4. Note your User ID from the success page
5. Use this ID for ESP32 configuration

### Syncing Activities

After OAuth, sync your activities:

```bash
curl -u admin:password \
  -X POST http://localhost:8080/admin/sync/{USER_ID}
```

## Deployment

### Docker (Recommended)

Create `Dockerfile`:

```dockerfile
FROM golang:1.21-alpine AS builder
WORKDIR /app
COPY go.* ./
RUN go mod download
COPY . .
RUN go build -o server ./cmd/server

FROM alpine:latest
RUN apk --no-cache add ca-certificates
WORKDIR /root/
COPY --from=builder /app/server .
EXPOSE 8080
CMD ["./server"]
```

Build and run:

```bash
docker build -t strava-server .
docker run -p 8080:8080 --env-file .env strava-server
```

### systemd Service

Create `/etc/systemd/system/strava-server.service`:

```ini
[Unit]
Description=Strava OAuth Server
After=network.target postgresql.service

[Service]
Type=simple
User=your-user
WorkingDirectory=/home/your-user/strava-server
Environment="PATH=/usr/local/go/bin:/usr/bin"
ExecStart=/home/your-user/strava-server/server
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start:

```bash
sudo systemctl daemon-reload
sudo systemctl enable strava-server
sudo systemctl start strava-server
sudo systemctl status strava-server
```

### Cloud Platforms

#### Fly.io

```bash
fly launch
fly secrets set STRAVA_CLIENT_ID=xxx STRAVA_CLIENT_SECRET=xxx ...
fly deploy
```

#### Railway

1. Push to GitHub
2. Connect Railway to repo
3. Add PostgreSQL database
4. Set environment variables
5. Deploy

## Troubleshooting

### Database Connection Failed

```bash
# Check PostgreSQL is running
sudo systemctl status postgresql

# Test connection
psql -U strava_user -d strava_db -h localhost
```

### Token Expired

Tokens are automatically refreshed when expired. If manual refresh is needed:

```bash
curl -u admin:password -X POST \
  http://localhost:8080/admin/sync/{USER_ID}
```

### OAuth Redirect Mismatch

Ensure `STRAVA_REDIRECT_URI` in `.env` exactly matches the redirect URI in your Strava app settings (including http/https).

### API Key Authentication Failed

Verify:
- API key in `.env` matches the key used in ESP32 config
- Header name is `X-API-Key` (case-sensitive)
- Key is sent with every API request

## Security Notes

- Never commit `.env` file
- Use strong passwords for database and admin
- Generate secure random API keys
- Use HTTPS in production
- Consider adding rate limiting for production
- Store tokens encrypted at rest (future enhancement)

## License

MIT

## Contributing

Pull requests welcome! Please ensure:
- Code follows Go conventions
- SQL queries are added to `db/queries/`
- Run `sqlc generate` after SQL changes
- Update README if adding new features

## Support

For issues or questions:
- Check existing GitHub issues
- Review Strava API documentation
- Check server logs for detailed errors
