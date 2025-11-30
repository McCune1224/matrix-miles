# Deploying Strava Server to Raspberry Pi

This guide explains how to deploy your Matrix Miles backend to a Raspberry Pi for local network access.

## Prerequisites

- Raspberry Pi (3, 4, or 5)
- Raspberry Pi OS Lite installed
- Network connectivity (same LAN as your dev machine and MatrixPortal)
- PostgreSQL database (either on the Pi or accessible from the Pi)

## Step 1: Determine Your Raspberry Pi Architecture

SSH into your Pi and run:
```bash
uname -m
```

- `aarch64` or `arm64` = Use `server-arm64` binary
- `armv7l` = Use `server-armv7` binary
- `armv6l` = Raspberry Pi Zero (needs separate build)

## Step 2: Transfer Binary to Raspberry Pi

From your development machine:
```bash
# For Pi 4/5 (64-bit)
scp /home/mckusa/Code/matrix-miles/strava-server/server-arm64 pi@<RASPBERRY_PI_IP>:~/

# For Pi 3 (32-bit)
scp /home/mckusa/Code/matrix-miles/strava-server/server-armv7 pi@<RASPBERRY_PI_IP>:~/
```

## Step 3: Set Up Environment on Raspberry Pi

SSH into your Pi:
```bash
ssh pi@<RASPBERRY_PI_IP>
```

Create a directory for the application:
```bash
mkdir -p /opt/strava-server
cd /opt/strava-server

# Copy the binary
sudo cp ~/ server-arm64 . (or server-armv7 for 32-bit)
sudo chmod +x server-arm64
```

Create an `.env` file with your configuration:
```bash
sudo tee /opt/strava-server/.env > /dev/null <<EOF
# Server Config
PORT=8080
DOMAIN=http://<RASPBERRY_PI_IP>:8080

# Strava OAuth (from your .env on dev machine)
STRAVA_CLIENT_ID=<your_client_id>
STRAVA_CLIENT_SECRET=<your_client_secret>
STRAVA_REDIRECT_URI=http://<RASPBERRY_PI_IP>:8080/auth/callback

# Database Config
DB_HOST=<database_ip_or_localhost>
DB_PORT=5432
DB_USER=strava_user
DB_PASSWORD=<your_db_password>
DB_NAME=strava_db
DB_SSLMODE=disable

# Security
ESP32_API_KEY=9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6
ADMIN_USERNAME=admin
ADMIN_PASSWORD=<your_admin_password>
EOF
```

## Step 4: Create a Systemd Service

This allows the server to start automatically on boot:

```bash
sudo tee /etc/systemd/system/strava-server.service > /dev/null <<EOF
[Unit]
Description=Matrix Miles Strava Server
After=network.target
Wants=network-online.target

[Service]
Type=simple
User=pi
WorkingDirectory=/opt/strava-server
EnvironmentFile=/opt/strava-server/.env
ExecStart=/opt/strava-server/server-arm64
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF
```

Enable and start the service:
```bash
sudo systemctl daemon-reload
sudo systemctl enable strava-server
sudo systemctl start strava-server
```

Check status:
```bash
sudo systemctl status strava-server
sudo journalctl -u strava-server -f  # View logs
```

## Step 5: Verify Network Access

From your development machine:
```bash
# Find Raspberry Pi on network
ping <RASPBERRY_PI_IP>

# Test the API endpoint
curl -H "X-API-Key: 9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6" \
  http://<RASPBERRY_PI_IP>:8080/api/activities/recent/1
```

You should get JSON response with activities.

## Step 6: Update MatrixPortal Configuration

Update the device settings.toml:

```toml
CIRCUITPY_WIFI_SSID = "black_mesa"
CIRCUITPY_WIFI_PASSWORD = "thecakeisalie!"
API_BASE_URL = "http://<RASPBERRY_PI_IP>:8080"
API_KEY = "9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6"
USER_ID = 1
REFRESH_INTERVAL_SECONDS = 60
```

## Troubleshooting

### Service won't start
```bash
# Check logs
sudo journalctl -u strava-server -n 50

# Check binary works
/opt/strava-server/server-arm64 --help
```

### Database connection errors
- Verify DB is running and accessible from Pi
- Check DB_HOST is correct (not localhost if DB is on different machine)
- Verify DB credentials in .env

### Network access issues
- Check Pi firewall: `sudo ufw status`
- Allow port 8080: `sudo ufw allow 8080/tcp`
- Verify Pi and MatrixPortal are on same network

### Binary won't run ("not found" error)
- You may have picked the wrong architecture
- Try running: `file server-arm64` to verify
- Or run on Pi: `./server-arm64` to see actual error

## Optional: Use HTTP over HTTPS with Self-Signed Cert

If you want HTTPS on the Pi later:

```bash
# Generate self-signed cert (on Pi)
cd /opt/strava-server
openssl req -x509 -newkey rsa:4096 -nodes -out cert.pem -keyout key.pem -days 365

# Update .env to use HTTPS
# DOMAIN=https://<RASPBERRY_PI_IP>:8443

# Update systemd service to use port 8443
```

For now, HTTP is simpler and works great on a local network.

## Summary

Your MatrixPortal will now:
1. Connect to WiFi ✓
2. Talk to your Raspberry Pi on the local network ✓
3. Fetch Strava activities via HTTP ✓
4. Display them on the LED matrix ✓

All without needing the internet or dealing with SSL certificates!
