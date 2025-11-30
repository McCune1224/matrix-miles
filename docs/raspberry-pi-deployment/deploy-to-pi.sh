#!/bin/bash
# deploy-to-pi.sh - Automated deployment to Raspberry Pi
# Usage: ./deploy-to-pi.sh <pi_ip> <architecture> <db_password> <strava_client_id> <strava_client_secret>

set -e

if [ $# -lt 5 ]; then
    echo "Usage: $0 <pi_ip> <architecture> <db_password> <strava_client_id> <strava_client_secret>"
    echo ""
    echo "Arguments:"
    echo "  pi_ip              - IP address of Raspberry Pi (e.g., 192.168.68.100)"
    echo "  architecture       - arm64 or armv7"
    echo "  db_password        - PostgreSQL password for strava_user"
    echo "  strava_client_id   - From Strava app registration"
    echo "  strava_client_secret - From Strava app registration"
    echo ""
    echo "Example:"
    echo "  $0 192.168.68.100 arm64 mydbpass 12345 abc123xyz"
    exit 1
fi

PI_IP=$1
ARCH=$2
DB_PASSWORD=$3
STRAVA_CLIENT_ID=$4
STRAVA_CLIENT_SECRET=$5

BINARY_NAME="server-${ARCH}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BINARY_PATH="${SCRIPT_DIR}/strava-server/${BINARY_NAME}"

echo "=== Matrix Miles Deployment to Raspberry Pi ==="
echo "Target: pi@${PI_IP}"
echo "Architecture: ${ARCH}"
echo "Binary: ${BINARY_NAME}"
echo ""

# Step 1: Verify binary exists
if [ ! -f "${BINARY_PATH}" ]; then
    echo "ERROR: Binary not found at ${BINARY_PATH}"
    echo "Building..."
    cd "${SCRIPT_DIR}/strava-server"
    GOOS=linux GOARCH=${ARCH/arm64/arm64} go build -o "${BINARY_NAME}" ./cmd/main.go
    echo "Build complete"
fi

# Step 2: Transfer binary
echo "Transferring binary to Pi..."
scp "${BINARY_PATH}" "pi@${PI_IP}:~/"

# Step 3: Set up environment on Pi
echo "Setting up application directory..."
ssh "pi@${PI_IP}" <<ENDSCRIPT
    set -e
    
    # Create directory
    sudo mkdir -p /opt/strava-server
    sudo chown pi:pi /opt/strava-server
    
    # Move binary
    mv ~/${BINARY_NAME} /opt/strava-server/
    chmod +x /opt/strava-server/${BINARY_NAME}
    
    # Create symlink for easy reference
    cd /opt/strava-server
    ln -sf ${BINARY_NAME} server || true
    
    echo "Application directory ready"
ENDSCRIPT

# Step 4: Create .env file
echo "Creating .env configuration..."
ssh "pi@${PI_IP}" <<ENDSCRIPT
    cat > /opt/strava-server/.env <<EOF
# Server Config
PORT=8080
DOMAIN=http://${PI_IP}:8080

# Strava OAuth
STRAVA_CLIENT_ID=${STRAVA_CLIENT_ID}
STRAVA_CLIENT_SECRET=${STRAVA_CLIENT_SECRET}
STRAVA_REDIRECT_URI=http://${PI_IP}:8080/auth/callback

# Database Config
DB_HOST=localhost
DB_PORT=5432
DB_USER=strava_user
DB_PASSWORD=${DB_PASSWORD}
DB_NAME=strava_db
DB_SSLMODE=disable

# Security
ESP32_API_KEY=9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6
ADMIN_USERNAME=admin
ADMIN_PASSWORD=changeme
EOF
    
    echo ".env file created"
ENDSCRIPT

# Step 5: Create systemd service
echo "Creating systemd service..."
ssh "pi@${PI_IP}" <<'ENDSCRIPT'
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
ExecStart=/opt/strava-server/server
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF
    
    sudo systemctl daemon-reload
    echo "Systemd service installed"
ENDSCRIPT

# Step 6: Enable and start service
echo "Starting service..."
ssh "pi@${PI_IP}" <<'ENDSCRIPT'
    sudo systemctl enable strava-server
    sudo systemctl start strava-server
    sleep 2
    sudo systemctl status strava-server
ENDSCRIPT

# Step 7: Test connectivity
echo ""
echo "Testing API connectivity..."
sleep 2
if curl -s -H "X-API-Key: 9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6" \
    "http://${PI_IP}:8080/api/activities/recent/1" > /dev/null 2>&1; then
    echo "✓ API is responding!"
else
    echo "⚠ API test failed (database may not be initialized yet)"
    echo "Check logs with: ssh pi@${PI_IP} sudo journalctl -u strava-server -f"
fi

echo ""
echo "=== Deployment Complete ==="
echo ""
echo "Next steps:"
echo "1. Update MatrixPortal settings.toml:"
echo "   API_BASE_URL = \"http://${PI_IP}:8080\""
echo ""
echo "2. Check logs:"
echo "   ssh pi@${PI_IP} sudo journalctl -u strava-server -f"
echo ""
echo "3. Manage service:"
echo "   ssh pi@${PI_IP} sudo systemctl restart strava-server"
