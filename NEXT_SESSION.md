# Next Session: ESP32 Setup

## What's Ready

✅ **Go Server** - Fully functional and tested
- Located in `strava-server/`
- Built binary: `strava-server/server`
- Run with: `./server` or `air` (hot reload)
- All credentials configured in `.env`

✅ **ESP32 MicroPython Code** - Ready to flash
- Located in `esp32-client/`
- All 4 files prepared: `config.py`, `boot.py`, `api_client.py`, `main.py`
- Comprehensive setup guide: `ESP32_SETUP_GUIDE.md`

## Your Next Steps

### 1. Get Your User ID (5 minutes)

You need your Strava User ID before setting up ESP32:

```bash
# Option A: Complete OAuth flow
cd ~/Code/micro-srava/strava-server
./server
# Then visit: http://localhost:8080/auth/login
# Note the User ID shown after authorizing

# Option B: Query database directly
psql "postgresql://postgres:nuzXwPdIMuEkqSWiMlFDDrJBzuXWDxoh@hopper.proxy.rlwy.net:37026/railway" -c "SELECT id, strava_user_id FROM users;"
```

### 2. Follow ESP32 Setup Guide

Open `ESP32_SETUP_GUIDE.md` and follow the step-by-step instructions:

1. **Install tools** (esptool, mpremote) - 5 min
2. **Flash MicroPython** to ESP32 - 10 min
3. **Update configuration** files - 5 min
4. **Upload files** to ESP32 - 5 min
5. **Test and run** - 5 min

**Total time: ~30 minutes**

## Quick Commands Reference

### Start Go Server
```bash
cd ~/Code/micro-srava/strava-server
./server
# Or with hot reload:
air
```

### Test Server Endpoints
```bash
# Health check
curl http://localhost:8080/health

# Get activities (replace USER_ID)
curl -H "X-API-Key: 9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6" \
  http://localhost:8080/api/activities/recent/USER_ID

# Get stats
curl -H "X-API-Key: 9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6" \
  http://localhost:8080/api/stats/USER_ID
```

### Flash ESP32 (Quick)
```bash
# Erase and flash (from Downloads directory after downloading firmware)
esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x1000 ESP32_GENERIC-20240602-v1.23.0.bin

# Upload files (from esp32-client directory)
cd ~/Code/micro-srava/esp32-client
mpremote connect /dev/ttyUSB0 mip install urequests
mpremote connect /dev/ttyUSB0 fs cp config.py :config.py
mpremote connect /dev/ttyUSB0 fs cp boot.py :boot.py
mpremote connect /dev/ttyUSB0 fs cp api_client.py :api_client.py
mpremote connect /dev/ttyUSB0 fs cp main.py :main.py

# Run
mpremote connect /dev/ttyUSB0
>>> import main
```

## Important Files

- **ESP32_SETUP_GUIDE.md** - Complete setup walkthrough
- **SETUP_COMPLETE.md** - Server setup documentation and credentials
- **esp32-client/** - All ESP32 code ready to upload
- **strava-server/.env** - Server configuration (DO NOT COMMIT)

## What You'll Need

Hardware:
- [ ] ESP32 board
- [ ] USB cable

Software (install if not present):
- [ ] Python 3.7+
- [ ] esptool: `pip3 install esptool`
- [ ] mpremote: `pip3 install mpremote`

Configuration:
- [ ] WiFi SSID and password
- [ ] Your Strava User ID (get from OAuth)
- [ ] Server IP address (for local) or URL (for cloud)

## Credentials Summary

```
ESP32 API Key: 9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6
Admin User: admin
Admin Pass: strava_admin_2025
Database: Railway PostgreSQL (already connected)
```

## After ESP32 Works

Future enhancements:
1. Add LED matrix display (MAX7219)
2. Implement calendar view on matrix
3. Add button controls
4. Deploy server to production
5. Optimize power consumption with deep sleep

## Need Help?

All troubleshooting steps are in **ESP32_SETUP_GUIDE.md**

Common issues:
- WiFi not connecting → Check SSID/password
- Can't reach server → Use your machine's IP, not localhost
- API key errors → Verify it matches .env file
- Import errors → Run `mpremote mip install urequests`

Good luck! The code is tested and ready to go.
