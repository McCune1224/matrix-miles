# Quick Start: Raspberry Pi Deployment

## TL;DR - Express Setup (10 minutes)

### On Your Development Machine

```bash
# 1. Build for Raspberry Pi (if not already done)
cd ~/Code/matrix-miles/strava-server
GOOS=linux GOARCH=arm64 go build -o server-arm64 ./cmd/main.go  # For Pi 4/5 (64-bit)
# OR
GOOS=linux GOARCH=arm GOARM=7 go build -o server-armv7 ./cmd/main.go  # For Pi 3 (32-bit)

# 2. Use the automated deployment script
chmod +x /tmp/deploy-to-pi.sh
/tmp/deploy-to-pi.sh 192.168.68.100 arm64 mydbpass STRAVA_CLIENT_ID STRAVA_CLIENT_SECRET
```

Replace:
- `192.168.68.100` with your Pi's IP
- `arm64` or `armv7` based on your Pi model
- `mydbpass` with your actual database password
- `STRAVA_CLIENT_ID` and `STRAVA_CLIENT_SECRET` with your Strava OAuth credentials

That's it! The script will:
- Transfer the binary
- Create `/opt/strava-server`
- Set up environment variables
- Create a systemd service
- Start the server
- Test the API

### On Your MatrixPortal

Update `settings.toml`:
```toml
API_BASE_URL = "http://192.168.68.100:8080"
```

That's it! The device should now fetch from your Pi.

---

## Detailed Setup (Manual)

If you prefer to do it step-by-step:

### 1. Check Your Pi

```bash
ssh pi@192.168.68.100
uname -m  # Note the output (aarch64 = arm64, armv7l = armv7)
```

### 2. Transfer Binary

```bash
scp server-arm64 pi@192.168.68.100:~/
```

### 3. Set Up on Pi

```bash
ssh pi@192.168.68.100

# Create directory
sudo mkdir -p /opt/strava-server
cd /opt/strava-server
sudo cp ~/server-arm64 .
sudo chmod +x server-arm64

# Create .env with your config
sudo tee .env > /dev/null <<EOF
PORT=8080
DOMAIN=http://192.168.68.100:8080
STRAVA_CLIENT_ID=your_id
STRAVA_CLIENT_SECRET=your_secret
STRAVA_REDIRECT_URI=http://192.168.68.100:8080/auth/callback
DB_HOST=localhost
DB_PORT=5432
DB_USER=strava_user
DB_PASSWORD=your_password
DB_NAME=strava_db
DB_SSLMODE=disable
ESP32_API_KEY=9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6
ADMIN_USERNAME=admin
ADMIN_PASSWORD=changeme
EOF

# Test it runs
./server-arm64  # Should show startup messages
# Ctrl+C to stop
```

### 4. Create Systemd Service

```bash
sudo tee /etc/systemd/system/strava-server.service > /dev/null <<EOF
[Unit]
Description=Matrix Miles Strava Server
After=network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/opt/strava-server
EnvironmentFile=/opt/strava-server/.env
ExecStart=/opt/strava-server/server-arm64
Restart=on-failure

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable strava-server
sudo systemctl start strava-server
```

### 5. Verify

```bash
# Check it's running
sudo systemctl status strava-server

# View logs
sudo journalctl -u strava-server -f

# Test API (from your dev machine)
curl -H "X-API-Key: 9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6" \
  http://192.168.68.100:8080/api/activities/recent/1
```

---

## Architecture Reference

- **Raspberry Pi 3**: armv7l → use `server-armv7`
- **Raspberry Pi 4**: aarch64 → use `server-arm64`
- **Raspberry Pi 5**: aarch64 → use `server-arm64`
- **Raspberry Pi Zero**: armv6l → needs separate build

Check with: `ssh pi@IP uname -m`

---

## Networking Tips

### Find Your Pi's IP

```bash
# On the Pi
hostname -I

# Or from your dev machine
arp-scan --interface=wlan0 --localnet | grep -i "raspberry\|b8:27:eb\|dc:a6:32"
```

### Allow Firewall Access

```bash
ssh pi@192.168.68.100
sudo ufw allow 8080/tcp
```

### Test Connectivity

```bash
# From dev machine or MatrixPortal
ping 192.168.68.100
curl http://192.168.68.100:8080/api/activities/recent/1
```

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| "Connection refused" | Service not running: `sudo systemctl start strava-server` |
| "Binary not found" | Wrong architecture: check `file server-arm64` |
| "Database error" | Check DB is running and accessible, verify DB_HOST in .env |
| "API key error" | Verify `ESP32_API_KEY` matches in code and settings |
| Service keeps crashing | Check logs: `sudo journalctl -u strava-server -n 50` |

---

## Performance Notes

Raspberry Pi should handle this workload easily:
- Go binary is lightweight (~16MB)
- API calls are simple queries
- 1-2 requests per minute from MatrixPortal
- Pi 3 onwards has plenty of capacity

For optimal performance:
- Use Raspberry Pi 4 or 5 (Pi 3 is slower but works fine)
- Ensure good WiFi signal
- Don't run resource-intensive tasks alongside this

---

## Security Notes

For local network only:
- HTTP is fine (same network, trusted devices)
- ESP32_API_KEY protects the endpoint
- Database should be on a secure machine

If exposing to internet later:
- Use HTTPS with self-signed cert
- Use firewall rules to restrict access
- Consider VPN for remote access
