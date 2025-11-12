# Migration Steps - Zap Logging Implementation

## Step-by-Step Deployment Guide

### 1. Run Database Migration
Apply the new migration to create the `application_logs` table:

```bash
# If using psql directly
psql -U your_user -d your_database -f db/migrations/002_application_logs.sql

# Or if using a migration tool
# Add the migration to your migration pipeline
```

### 2. Verify Migration
Check that the table was created:

```sql
\d application_logs
-- Should show the table structure

SELECT * FROM application_logs LIMIT 1;
-- Should return empty result (no errors)
```

### 3. Build the Application
```bash
cd strava-server
go build -o strava-server ./cmd/server
```

### 4. Update Environment Variables (if needed)
No new environment variables are required for the basic setup.

Optional: If you want to configure the log flush interval, you can add it to your config.

### 5. Test Locally First
```bash
# Start the server
./strava-server

# You should see colored Zap logs in the console:
# 2025-11-11T21:20:15.000Z  INFO  Database connected successfully
# 2025-11-11T21:20:15.001Z  INFO  Log flusher started  {"schedule": "*/5 * * * *"}
# 2025-11-11T21:20:15.002Z  INFO  Server starting  {"address": ":8080", "oauth_url": "..."}
```

### 6. Generate Some Logs
Make a few API requests to generate logs:

```bash
# Health check
curl http://localhost:8080/health

# Try an API endpoint (will fail without auth, but generates logs)
curl http://localhost:8080/api/activities/recent/1
```

### 7. Wait for Cron Flush (or Restart)
Option A: Wait 5 minutes for the cron job to flush logs
Option B: Restart the server (triggers graceful shutdown flush)

### 8. Verify Logs in Database
```sql
SELECT 
    id, 
    level, 
    message, 
    timestamp, 
    caller,
    fields 
FROM application_logs 
ORDER BY timestamp DESC 
LIMIT 10;
```

You should see logs like:
- "Request completed" with method, path, status, duration
- "Server starting" with address and oauth_url
- "Database connected successfully"

### 9. Test Admin Endpoints
```bash
# Get recent logs
curl -u admin:password http://localhost:8080/admin/logs

# Get error logs only
curl -u admin:password http://localhost:8080/admin/logs/level/error

# Should return JSON array of log entries
```

### 10. Monitor Performance
Keep an eye on:
- Database size of `application_logs` table
- Memory usage (logs buffered in memory)
- CPU usage during cron flushes

## Rollback Plan

If you need to rollback:

1. Stop the new server
2. Start the old server version
3. Optionally drop the logs table:
```sql
DROP TABLE IF EXISTS application_logs CASCADE;
DROP FUNCTION IF EXISTS cleanup_old_logs();
```

## Production Considerations

### 1. Change to Production Mode
In `main.go`, change:
```go
log, err := logger.NewLogger(queries, false) // false = production mode
```

This will:
- Set log level to INFO (no DEBUG logs)
- Remove color codes from console output

### 2. Adjust Cron Schedule
For high-traffic production:
```go
// Flush every 1 minute instead of 5
logFlusher.Start("*/1 * * * *")
```

### 3. Monitor Database Growth
Set up monitoring for the `application_logs` table size.

Expected growth:
- ~1KB per log entry
- 1000 requests/day = ~1MB/day
- With 30-day retention = ~30MB

### 4. Consider Log Retention
The default is 30 days. Adjust in the migration file if needed:
```sql
DELETE FROM application_logs 
WHERE timestamp < NOW() - INTERVAL '7 days'; -- Weekly retention
```

### 5. Set Up Periodic Cleanup Cron
Add a database cron job to run cleanup daily:
```sql
-- Using pg_cron extension
SELECT cron.schedule('cleanup-old-logs', '0 2 * * *', 'SELECT cleanup_old_logs()');
```

## Troubleshooting

### Logs not appearing in database
- Check server logs for flush errors
- Verify database connection
- Check buffer size (may need to trigger more logs to fill buffer)
- Try restarting server to force flush

### High memory usage
- Reduce buffer size in `pkg/logger/logger.go`
- Increase flush frequency

### Slow database writes
- Check database performance
- Verify indexes are created
- Consider partitioning the logs table by date

### Permission errors on admin endpoints
- Verify basic auth credentials in .env file
- Check that ADMIN_USERNAME and ADMIN_PASSWORD are set

## Success Criteria

✅ Database migration applied successfully  
✅ Server starts without errors  
✅ Logs appear in console with colors (dev mode)  
✅ Logs are written to database after 5 minutes or restart  
✅ Admin endpoints return log data  
✅ No performance degradation  

## Support

For issues or questions, check:
- Server logs (console output)
- Database logs
- `ZAP_LOGGING_IMPLEMENTATION.md` for detailed documentation
