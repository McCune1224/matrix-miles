# Zap Logging Implementation

## Overview
Successfully implemented Zap structured logging with periodic database writes via cron job for the Strava server.

## What Was Implemented

### 1. Dependencies Added
- `go.uber.org/zap` - Structured logging library
- `github.com/robfig/cron/v3` - Cron job scheduler

### 2. Database Schema
**New Migration**: `db/migrations/002_application_logs.sql`
- `application_logs` table with fields:
  - `id` (BIGSERIAL) - Primary key
  - `level` (VARCHAR) - Log level (debug, info, warn, error, fatal)
  - `message` (TEXT) - Log message
  - `timestamp` (TIMESTAMP) - When the log occurred
  - `caller` (VARCHAR) - Source file and line number
  - `stack_trace` (TEXT) - Stack trace for errors
  - `fields` (JSONB) - Structured fields (user_id, activity_id, etc.)
  - `created_at` (TIMESTAMP) - When record was created
- Indexes for efficient querying by timestamp, level, and JSONB fields
- Auto-cleanup function for logs older than 30 days

### 3. Database Queries
**New File**: `db/queries/logs.sql`
- `InsertLogBatch` - Batch insert logs (using COPY FROM for performance)
- `GetRecentLogs` - Get logs from last N hours
- `GetLogsByLevel` - Filter logs by level
- `GetLogsByUserID` - Filter logs by user_id in JSONB fields
- `CleanupOldLogs` - Delete old logs

### 4. Logger Package
**New Package**: `pkg/logger/logger.go`
- Custom `DatabaseSink` that buffers logs in memory
- Dual output: console (colored, development-friendly) + database (JSON)
- Thread-safe buffer with configurable size (100 logs)
- Automatic flush when buffer is full
- Manual flush capability for graceful shutdown

Key Features:
- Structured logging with typed fields
- Caller information (file:line)
- Stack traces for errors
- JSONB storage for rich querying

### 5. Cron Service
**New Service**: `internal/services/log_flusher.go`
- Periodic log flushing to database
- Configurable schedule (default: every 5 minutes)
- Graceful shutdown with final flush
- Error handling for failed flushes

### 6. HTTP Request Logging
**New Middleware**: `zapRequestLogger` in `cmd/server/main.go`
- Logs all HTTP requests with:
  - Method, path, status code
  - Request duration
  - Client IP
  - Errors (if any)
- Replaces Echo's default logger middleware

### 7. Admin API Endpoints
**New Handler**: `internal/handlers/logs.go`

Three new admin endpoints (require basic auth):
- `GET /admin/logs?hours=24&limit=100` - Get recent logs
- `GET /admin/logs/level/:level?hours=24&limit=100` - Filter by level
- `GET /admin/logs/user/:userId?hours=24&limit=100` - Filter by user

### 8. Main Application Updates
**Updated**: `cmd/server/main.go`
- Initialize Zap logger on startup
- Start cron job for periodic log flushing
- Graceful shutdown with log flushing
- Replace standard library logging with Zap

## Usage

### Running the Server
```bash
# Run database migration first
# Then start the server
go run ./cmd/server/main.go
```

### Logging in Code
```go
// In handlers or services, use the global logger
import "go.uber.org/zap"

log.Info("User logged in", 
    zap.Int32("user_id", userID),
    zap.String("username", username))

log.Error("Failed to sync activities", 
    zap.Error(err),
    zap.Int32("user_id", userID))
```

### Viewing Logs
```bash
# Get recent logs (last 24 hours)
curl -u admin:password http://localhost:8080/admin/logs

# Get error logs only
curl -u admin:password http://localhost:8080/admin/logs/level/error

# Get logs for specific user
curl -u admin:password http://localhost:8080/admin/logs/user/123?hours=48&limit=50
```

### Database Query Examples
```sql
-- Get all error logs from today
SELECT * FROM application_logs 
WHERE level = 'error' 
  AND timestamp >= CURRENT_DATE
ORDER BY timestamp DESC;

-- Get logs for specific user
SELECT * FROM application_logs 
WHERE fields->>'user_id' = '123'
ORDER BY timestamp DESC;

-- Get logs with specific activity
SELECT * FROM application_logs 
WHERE fields->>'activity_id' IS NOT NULL
ORDER BY timestamp DESC;

-- Clean up old logs manually
SELECT cleanup_old_logs();
```

## Configuration

### Cron Schedule
Change the schedule in `main.go`:
```go
// Every 5 minutes (default)
logFlusher.Start("*/5 * * * *")

// Every 15 minutes
logFlusher.Start("*/15 * * * *")

// Every hour
logFlusher.Start("0 * * * *")
```

### Log Buffer Size
Adjust in `pkg/logger/logger.go`:
```go
dbSink := NewDatabaseSink(queries, 200) // Buffer 200 logs instead of 100
```

### Log Level
Change in `main.go`:
```go
// Production mode (Info level and above)
log, err := logger.NewLogger(queries, false)

// Development mode (Debug level and above)
log, err := logger.NewLogger(queries, true)
```

## Benefits

1. **Structured Logging**: Rich, queryable log data with typed fields
2. **Performance**: Batch inserts reduce database load
3. **Observability**: Query logs by user, level, time range, or any JSONB field
4. **Persistence**: Logs survive application restarts
5. **Retention**: Automatic cleanup of old logs (30 days)
6. **Development**: Beautiful colored console output
7. **Production**: JSON logs written to database for analysis

## Next Steps (Optional Enhancements)

1. Add log streaming endpoint (WebSocket or SSE)
2. Implement log aggregation dashboard
3. Add alerting for error thresholds
4. Export logs to external services (e.g., DataDog, Sentry)
5. Add log search with full-text indexing
6. Create retention policy configuration
7. Add log statistics endpoint (counts by level, etc.)

## Files Modified
- `strava-server/go.mod` - Added dependencies
- `strava-server/cmd/server/main.go` - Integrated logger and middleware

## Files Created
- `strava-server/db/migrations/002_application_logs.sql`
- `strava-server/db/queries/logs.sql`
- `strava-server/pkg/logger/logger.go`
- `strava-server/internal/services/log_flusher.go`
- `strava-server/internal/handlers/logs.go`
- `strava-server/internal/database/logs.sql.go` (generated by sqlc)
- `strava-server/internal/database/copyfrom.go` (generated by sqlc)

## Testing
All code compiles successfully. To test:
1. Run the database migration: `002_application_logs.sql`
2. Start the server
3. Make some API requests
4. Wait 5 minutes or restart server to flush logs
5. Query logs via admin endpoints or database
