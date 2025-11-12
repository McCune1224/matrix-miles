# Strava Server Setup - COMPLETE ✓

## What Was Done

### 1. Fixed Dependencies
- Installed `pgx/v5` packages (required by sqlc-generated code)
- Removed `sqlx` dependency in favor of direct `pgxpool` usage
- Fixed all type conversions between Go types and `pgtype` types

### 2. Database Configuration
- Applied migrations to Railway PostgreSQL
- Created `users` and `activities` tables with proper indexes
- Database is fully configured and accessible

### 3. Server Configuration
- Created `.env` file in `strava-server/` directory with all credentials
- Generated secure ESP32 API key
- Configured Strava OAuth credentials
- Set up Railway PostgreSQL connection

### 4. Testing
- Server builds successfully: `strava-server/server`
- Database connection works
- Health endpoint responding: `http://localhost:8080/health`
- API key authentication working correctly

---

## Important Credentials

### ESP32 API Key
```
9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6
```

### Admin Credentials
- Username: `admin`
- Password: `strava_admin_2025`

### Strava OAuth
- Client ID: `184585`
- Client Secret: `57f1135bb297a76d0f0bdedc855c16851c99c5d7`
- Redirect URI: `http://localhost:8080/auth/callback`

### Database (Railway PostgreSQL)
- Host: `hopper.proxy.rlwy.net`
- Port: `37026`
- User: `postgres`
- Database: `railway`
- Connection String: `postgresql://postgres:nuzXwPdIMuEkqSWiMlFDDrJBzuXWDxoh@hopper.proxy.rlwy.net:37026/railway?sslmode=require`

---

## Running the Server

### Start Server (Production)
```bash
cd strava-server
./server
```

### Start Server (Development with Hot Reload)
```bash
cd strava-server
air
```

Air will automatically rebuild and restart when you modify code. See `strava-server/DEVELOPMENT.md` for details.

You should see:
```
✓ Database connected successfully
🚀 Server starting on :8080
📍 OAuth login: http://localhost:8080/auth/login
```

### Run in Background
```bash
cd strava-server
nohup ./server > server.log 2>&1 &
```

### Check Logs
```bash
tail -f strava-server/server.log
```

### Stop Server
```bash
pkill -f "./server"
# Or if using Air:
pkill -f "tmp/main"
```

---

## Next Steps

### 1. Complete OAuth Flow
1. Start the server
2. Visit: `http://localhost:8080/auth/login`
3. Authorize with Strava
4. Note your **User ID** from the success page

### 2. Sync Your Activities
```bash
curl -u admin:strava_admin_2025 \
  -X POST http://localhost:8080/admin/sync/{YOUR_USER_ID}
```

Replace `{YOUR_USER_ID}` with the ID from step 1.

### 3. Test API Endpoints

#### Get Recent Activities
```bash
curl -H "X-API-Key: 9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6" \
  http://localhost:8080/api/activities/recent/{YOUR_USER_ID}
```

#### Get Calendar Data
```bash
curl -H "X-API-Key: 9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6" \
  http://localhost:8080/api/activities/calendar/{YOUR_USER_ID}/2025/11
```

#### Get Stats
```bash
curl -H "X-API-Key: 9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6" \
  http://localhost:8080/api/stats/{YOUR_USER_ID}
```

### 4. Configure ESP32 (Optional)
See **`ESP32_SETUP_GUIDE.md`** for the complete step-by-step ESP32 setup guide.
All ESP32 MicroPython code is ready in the `esp32-client/` directory.

---

## API Endpoints Summary

### Public Endpoints
- `GET /health` - Health check
- `GET /auth/login` - Start OAuth flow
- `GET /auth/callback` - OAuth callback (don't call directly)

### Protected Endpoints (Require `X-API-Key` header)
- `GET /api/activities/recent/:userId` - Recent activities
- `GET /api/activities/calendar/:userId/:year/:month` - Calendar data
- `GET /api/stats/:userId` - User statistics

### Admin Endpoints (Require Basic Auth)
- `POST /admin/sync/:userId` - Sync activities from Strava

---

## Troubleshooting

### Server Won't Start
```bash
# Check if port 8080 is already in use
lsof -i :8080
# Kill any process using port 8080
kill -9 $(lsof -t -i :8080)
```

### Database Connection Issues
```bash
# Test database connection
psql "postgresql://postgres:nuzXwPdIMuEkqSWiMlFDDrJBzuXWDxoh@hopper.proxy.rlwy.net:37026/railway"
```

### OAuth Issues
- Ensure redirect URI in Strava app settings matches: `http://localhost:8080/auth/callback`
- For production, update redirect URI to your domain

### API Key Not Working
- Check the key in `.env` file: `strava-server/.env`
- Ensure header name is exactly: `X-API-Key` (case-sensitive)

---

## Project Structure

```
/home/mckusa/Code/micro-srava/
├── strava-server/              # Go HTTP server
│   ├── cmd/server/main.go      # Entry point
│   ├── internal/
│   │   ├── database/           # sqlc generated code
│   │   ├── handlers/           # HTTP handlers
│   │   └── strava/             # Strava API client
│   ├── db/
│   │   ├── migrations/         # SQL schema
│   │   └── queries/            # SQL queries
│   ├── .env                    # Configuration (DO NOT COMMIT)
│   └── server                  # Compiled binary
├── micropython-setup.md        # ESP32 setup guide
└── SETUP_COMPLETE.md           # This file
```

---

## Security Notes

- The `.env` file contains sensitive credentials - DO NOT commit to git
- Change admin password before deploying to production
- Use HTTPS in production (update DOMAIN in .env)
- Consider adding rate limiting for production use
- The API key should be kept secure and not shared

---

## Production Deployment

For production deployment:
1. Update `DOMAIN` in `.env` to your production URL
2. Update Strava redirect URI to production URL
3. Use HTTPS (required by Strava OAuth)
4. Set up systemd service or use Docker (see README.md)
5. Configure firewall rules
6. Set up monitoring and logging

See `strava-server/README.md` for detailed deployment instructions.

---

## Support

For issues:
- Check server logs: `strava-server/server.log`
- Check database tables: `psql ... -c "\dt"`
- Verify environment variables: `cat strava-server/.env`
- Review API documentation: `strava-server/README.md`
