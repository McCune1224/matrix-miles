# Railway Deployment Guide

This guide will help you deploy your Strava Server to Railway.app with PostgreSQL from the monorepo.

## Prerequisites

- Railway account (sign up at https://railway.app)
- GitHub account (for connecting your repository)
- Strava API credentials (from https://www.strava.com/settings/api)

## Project Structure

This is a monorepo with multiple projects:
```
matrix-miles/
├── Dockerfile              # Root Dockerfile that builds strava-server
├── docker-compose.yml      # Local development setup
├── railway.json            # Railway configuration
├── .dockerignore          # Docker build exclusions
├── strava-server/         # Go server application
├── esp32_client_cpp/      # ESP32 client code
└── ...
```

The Dockerfile at the root builds the strava-server subdirectory, allowing Railway to deploy from a single repository.

## Deployment Steps

### 1. Push Your Code to GitHub

```bash
cd /home/mckusa/Code/matrix-miles

# Initialize git if not already done
git init
git add .
git commit -m "Add Railway deployment configuration"

# Push to GitHub (replace with your repo URL)
git remote add origin https://github.com/YOUR_USERNAME/matrix-miles.git
git push -u origin main
```

### 2. Create a New Railway Project

1. Go to https://railway.app/new
2. Click "Deploy from GitHub repo"
3. Select your `matrix-miles` repository
4. Railway will detect the Dockerfile at the root automatically
   - The Dockerfile is configured to build only the strava-server subdirectory
   - No additional configuration needed!

### 3. Add PostgreSQL Database

1. In your Railway project dashboard, click "+ New"
2. Select "Database" → "Add PostgreSQL"
3. Railway will provision a PostgreSQL database and automatically set the `DATABASE_URL` environment variable

### 4. Configure Environment Variables

In your Railway project, go to the service settings and add these variables:

#### Required Variables:

```bash
# Server Configuration
PORT=8080
DOMAIN=https://your-app-name.up.railway.app

# Strava API (get from https://www.strava.com/settings/api)
STRAVA_CLIENT_ID=your_client_id_here
STRAVA_CLIENT_SECRET=your_client_secret_here
STRAVA_REDIRECT_URI=https://your-app-name.up.railway.app/auth/callback

# Database (Railway auto-provides DATABASE_URL, but we need individual components)
# Get these from the PostgreSQL service variables:
DB_HOST=${{Postgres.PGHOST}}
DB_PORT=${{Postgres.PGPORT}}
DB_USER=${{Postgres.PGUSER}}
DB_PASSWORD=${{Postgres.PGPASSWORD}}
DB_NAME=${{Postgres.PGDATABASE}}
DB_SSLMODE=require

# Security
ESP32_API_KEY=your_secure_api_key_here
ADMIN_USERNAME=admin
ADMIN_PASSWORD=your_secure_admin_password
```

#### Generate Secure Keys:

```bash
# Generate ESP32 API Key
openssl rand -hex 32

# Generate Admin Password
openssl rand -base64 24
```

### 5. Update Strava API Settings

1. Go to https://www.strava.com/settings/api
2. Update the **Authorization Callback Domain** to your Railway domain:
   - Example: `your-app-name.up.railway.app`
3. Save changes

### 6. Run Database Migrations

After the first deployment, you need to run the database migrations:

#### Option A: Using Railway CLI (Recommended)

```bash
# Install Railway CLI
npm i -g @railway/cli

# Login
railway login

# Link to your project
railway link

# Connect to PostgreSQL and run migrations
railway run psql $DATABASE_URL -f db/migrations/001_initial_schema.sql
railway run psql $DATABASE_URL -f db/migrations/002_application_logs.sql
```

#### Option B: Using Railway Dashboard

1. Go to your PostgreSQL service in Railway
2. Click "Data" tab
3. Click "Query"
4. Copy and paste the contents of `db/migrations/001_initial_schema.sql`
5. Execute
6. Repeat for `002_application_logs.sql`

### 7. Deploy and Test

1. Railway will automatically deploy after you push to GitHub
2. Monitor the deployment logs in the Railway dashboard
3. Once deployed, test the health endpoint:

```bash
curl https://your-app-name.up.railway.app/health
```

Expected response:
```json
{
  "status": "ok",
  "time": "2025-11-14T12:00:00Z"
}
```

### 8. Test OAuth Flow

1. Visit: `https://your-app-name.up.railway.app/auth/login`
2. Authorize with Strava
3. Note your User ID from the success message
4. Use this User ID in your ESP32 configuration

### 9. Sync Your Activities

```bash
# Replace YOUR_USER_ID with the ID from step 8
curl -u admin:your_admin_password \
  -X POST \
  https://your-app-name.up.railway.app/admin/sync/YOUR_USER_ID
```

## Testing Locally with Docker

Before deploying, you can test locally from the root of the repository:

```bash
# From the matrix-miles root directory
cd /home/mckusa/Code/matrix-miles

# Build and run with docker-compose
docker-compose up --build

# Or build and run manually
docker build -t strava-server .
docker run -p 8080:8080 --env-file strava-server/.env strava-server

# To use environment variables from strava-server/.env
docker run -p 8080:8080 --env-file strava-server/.env strava-server
```

The Dockerfile at the root is configured to:
- Copy only the strava-server subdirectory
- Exclude ESP32 and other unrelated files (via .dockerignore)
- Build the Go server from strava-server/cmd/main.go

## Environment Variables Reference

### Railway-Specific Variables

Railway provides these automatically:
- `PORT` - Railway assigns this dynamically
- `RAILWAY_ENVIRONMENT` - "production" or "development"
- `RAILWAY_GIT_COMMIT_SHA` - Current git commit

### Database Connection Variables

Use Railway's built-in variable references:
```bash
DB_HOST=${{Postgres.PGHOST}}
DB_PORT=${{Postgres.PGPORT}}
DB_USER=${{Postgres.PGUSER}}
DB_PASSWORD=${{Postgres.PGPASSWORD}}
DB_NAME=${{Postgres.PGDATABASE}}
```

## Monitoring and Logs

### View Logs

```bash
# Using Railway CLI
railway logs

# Or view in Railway dashboard under "Deployments" → "View Logs"
```

### Check Application Health

```bash
curl https://your-app-name.up.railway.app/health
```

### View Application Logs (Admin)

```bash
# Get recent logs
curl -u admin:password \
  https://your-app-name.up.railway.app/admin/logs

# Get logs by level
curl -u admin:password \
  https://your-app-name.up.railway.app/admin/logs/level/error
```

## Troubleshooting

### Deployment Failed

**Check logs:**
```bash
railway logs
```

**Common issues:**
- Missing environment variables
- Database connection failed (check DB_* variables)
- Port binding issues (ensure app uses `PORT` env var)

### Database Connection Failed

**Verify database is running:**
```bash
railway run psql $DATABASE_URL -c "SELECT 1"
```

**Check SSL mode:**
- Railway requires `DB_SSLMODE=require`
- Local development uses `DB_SSLMODE=disable`

### OAuth Redirect Mismatch

1. Ensure `DOMAIN` matches your Railway URL exactly
2. Update Strava API callback domain
3. Redeploy after changes

### API Key Authentication Failed

1. Ensure `ESP32_API_KEY` is set in Railway
2. Verify the same key is used in your ESP32 code
3. Check header is `X-API-Key` (case-sensitive)

## Continuous Deployment

Railway automatically deploys when you push to your connected branch:

```bash
# Make changes
git add .
git commit -m "Update feature"
git push origin main

# Railway will automatically:
# 1. Build the Docker image
# 2. Run tests (if configured)
# 3. Deploy to production
# 4. Health check
```

## Scaling and Performance

### Vertical Scaling
- Railway allows you to adjust memory and CPU in project settings
- Recommended: Start with 1GB RAM, 1 vCPU

### Horizontal Scaling
- Enable multiple replicas in `railway.json`
- Note: Session state is currently in-memory (future: Redis)

### Database Optimization
- Indexes are created automatically via migrations
- Monitor query performance in Railway PostgreSQL dashboard
- Consider upgrading database plan for more connections

## Cost Estimation

Railway pricing (as of 2025):
- **Hobby Plan**: $5/month + usage
  - Suitable for development/testing
- **Pro Plan**: $20/month + usage
  - Recommended for production
  - Better performance and uptime

Estimated monthly cost:
- Server: ~$5-10
- PostgreSQL: ~$5-15
- **Total: ~$10-25/month**

## Security Checklist

- ✅ Use strong, unique passwords
- ✅ Enable SSL for database (Railway default)
- ✅ Store secrets in Railway environment variables (never in code)
- ✅ Use HTTPS for all production endpoints (Railway default)
- ✅ Regularly rotate API keys and passwords
- ✅ Monitor logs for suspicious activity
- ✅ Keep dependencies updated

## Support

- Railway Docs: https://docs.railway.app
- Railway Discord: https://discord.gg/railway
- Strava API Docs: https://developers.strava.com

## Next Steps

1. ✅ Set up monitoring/alerting
2. ✅ Configure automatic backups for PostgreSQL
3. ✅ Add rate limiting for production
4. ✅ Set up custom domain (optional)
5. ✅ Configure ESP32 with production URL and API key
