# 🚂 Railway Deployment - Quick Guide

Deploy the strava-server from this monorepo to Railway.app in minutes.

## ✅ Pre-Deployment Checklist

- [ ] Strava API credentials ready (https://www.strava.com/settings/api)
- [ ] GitHub account with this repo pushed
- [ ] Railway.app account (free tier available)

## 🚀 Deployment Steps

### 1. Push to GitHub

```bash
cd /home/mckusa/Code/matrix-miles

git add .
git commit -m "Ready for Railway deployment"
git push origin main
```

### 2. Deploy to Railway

1. Go to https://railway.app/new
2. Click **"Deploy from GitHub repo"**
3. Select **`matrix-miles`** repository
4. Railway auto-detects the `Dockerfile` at root ✅

### 3. Add PostgreSQL

1. In Railway dashboard, click **"+ New"**
2. Select **"Database"** → **"Add PostgreSQL"**
3. Done! Railway auto-creates `DATABASE_URL`

### 4. Set Environment Variables

Click on your service → **"Variables"** tab:

```bash
PORT=8080
DOMAIN=https://your-app-name.up.railway.app
STRAVA_CLIENT_ID=your_client_id
STRAVA_CLIENT_SECRET=your_client_secret
STRAVA_REDIRECT_URI=https://your-app-name.up.railway.app/auth/callback
DB_HOST=${{Postgres.PGHOST}}
DB_PORT=${{Postgres.PGPORT}}
DB_USER=${{Postgres.PGUSER}}
DB_PASSWORD=${{Postgres.PGPASSWORD}}
DB_NAME=${{Postgres.PGDATABASE}}
DB_SSLMODE=require
ESP32_API_KEY=your_secure_api_key
ADMIN_USERNAME=admin
ADMIN_PASSWORD=your_secure_password
```

**Generate secure keys:**
```bash
openssl rand -hex 32    # ESP32_API_KEY
openssl rand -base64 24 # ADMIN_PASSWORD
```

### 5. Run Database Migrations

Install Railway CLI:
```bash
npm i -g @railway/cli
railway login
railway link
```

Run migrations:
```bash
railway run psql $DATABASE_URL -f strava-server/db/migrations/001_initial_schema.sql
railway run psql $DATABASE_URL -f strava-server/db/migrations/002_application_logs.sql
```

### 6. Update Strava API Settings

1. Go to https://www.strava.com/settings/api
2. Set **Authorization Callback Domain**: `your-app-name.up.railway.app`
3. Save

### 7. Test Deployment

```bash
# Health check
curl https://your-app-name.up.railway.app/health

# OAuth flow
open https://your-app-name.up.railway.app/auth/login
```

## 📱 Configure ESP32

After successful deployment, update your ESP32 code in `esp32_client_cpp/blink/blink.ino`:

```cpp
#define ESP32_API_KEY "your_railway_api_key"
const char* SERVER_URL = "https://your-app-name.up.railway.app";
```

See `strava-server/ESP32_PRODUCTION_CONFIG.md` for complete ESP32 setup.

## 🧪 Local Testing (Optional)

Test the Docker setup locally before deploying:

```bash
# From repository root
docker-compose up --build

# Server will start at http://localhost:8080
```

## 📚 Documentation

- **Complete Guide**: `strava-server/RAILWAY_DEPLOYMENT.md`
- **ESP32 Setup**: `strava-server/ESP32_PRODUCTION_CONFIG.md`
- **Server Docs**: `strava-server/README.md`
- **Project README**: `README.md`

## 🔍 Troubleshooting

**Build fails:**
```bash
# Check Railway logs
railway logs
```

**Can't connect to database:**
- Verify all `DB_*` variables are set correctly
- Check PostgreSQL service is running in Railway dashboard

**OAuth redirect fails:**
- Ensure `DOMAIN` matches your Railway URL exactly
- Verify Strava API callback domain is correct

**API key authentication fails:**
- Same key must be in Railway env vars and ESP32 code
- Header must be `X-API-Key` (case-sensitive)

## 💰 Cost Estimate

Railway pricing (as of 2025):
- Hobby Plan: $5/month + usage (~$5-10/month)
- Pro Plan: $20/month (recommended for production)

**Estimated total: $10-25/month**

## ✨ What's Included

The root Dockerfile builds only the strava-server:
- ✅ Multi-stage build (optimized size)
- ✅ Non-root user (security)
- ✅ Health checks
- ✅ Alpine Linux (minimal)
- ✅ Excludes ESP32/docs via .dockerignore

## 🎯 Quick Commands

```bash
# View logs
railway logs -f

# Run migrations
railway run psql $DATABASE_URL -f strava-server/db/migrations/001_initial_schema.sql

# SSH into container
railway run bash

# Redeploy
git push origin main
```

---

**Ready to deploy?** Follow the steps above or see the complete guide in `strava-server/RAILWAY_DEPLOYMENT.md`!
