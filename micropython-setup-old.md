# MicroPython Setup Guide for ESP32
## Strava OAuth Server & PostgreSQL Integration

This guide walks through setting up MicroPython on ESP32 to handle OAuth authentication flows and connect to a remote PostgreSQL database.

---

## Prerequisites

### Hardware
- ESP32 development board (ESP32-WROOM, ESP32-DevKitC, etc.)
- USB cable for programming
- Computer with USB port

### Software
- Python 3.7+ installed on your computer
- `esptool.py` for flashing firmware
- Serial terminal (screen, minicom, or Thonny IDE)

### Install esptool
```bash
pip3 install esptool
```

---

## Step 1: Download MicroPython Firmware

1. Visit [MicroPython Downloads](https://micropython.org/download/esp32/)
2. Download the latest stable `.bin` file for ESP32
   - For most ESP32 boards: `esp32-xxxxxx.bin`
   - For ESP32-S2/S3/C3: Download the specific variant

```bash
# Example: Download using wget
wget https://micropython.org/resources/firmware/esp32-20231005-v1.21.0.bin
```

---

## Step 2: Erase ESP32 Flash (First Time Setup)

1. Connect ESP32 to your computer via USB
2. Find the serial port:
   - Linux: `/dev/ttyUSB0` or `/dev/ttyACM0`
   - macOS: `/dev/cu.usbserial-*`
   - Windows: `COM3`, `COM4`, etc.

3. Erase flash memory:
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash
```

---

## Step 3: Flash MicroPython Firmware

```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x1000 esp32-20231005-v1.21.0.bin
```

**Note**: Adjust `--port` and firmware filename as needed.

Wait for "Hash of data verified" message indicating successful flash.

---

## Step 4: Connect to MicroPython REPL

### Option A: Using screen (Linux/macOS)
```bash
screen /dev/ttyUSB0 115200
```
Exit: `Ctrl+A` then `K`

### Option B: Using Thonny IDE
1. Install Thonny: `sudo apt install thonny` or download from [thonny.org](https://thonny.org)
2. Open Thonny → Run → Select interpreter → MicroPython (ESP32)
3. Select correct COM port

### Test Installation
In the REPL, type:
```python
>>> print("Hello from MicroPython!")
>>> import sys
>>> sys.implementation
```

---

## Step 5: WiFi Configuration

Create a `boot.py` file to auto-connect to WiFi on startup:

```python
# boot.py
import network
import time

def connect_wifi(ssid, password, timeout=10):
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    
    if not wlan.isconnected():
        print('Connecting to WiFi...')
        wlan.connect(ssid, password)
        
        start = time.time()
        while not wlan.isconnected():
            if time.time() - start > timeout:
                print('WiFi connection timeout!')
                return False
            time.sleep(0.5)
    
    print('WiFi connected!')
    print('IP address:', wlan.ifconfig()[0])
    return True

# Replace with your credentials
SSID = 'YOUR_WIFI_SSID'
PASSWORD = 'YOUR_WIFI_PASSWORD'

connect_wifi(SSID, PASSWORD)
```

---

## Step 6: HTTP Server for OAuth Callback

MicroPython doesn't have a built-in HTTP server library, so we'll create a simple one using sockets.

Create `oauth_server.py`:

```python
# oauth_server.py
import socket
import json

class SimpleHTTPServer:
    def __init__(self, port=8080):
        self.port = port
        self.auth_code = None
        
    def start(self):
        addr = socket.getaddrinfo('0.0.0.0', self.port)[0][-1]
        s = socket.socket()
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(addr)
        s.listen(1)
        
        print(f'OAuth server listening on port {self.port}')
        print(f'Redirect URI: http://<ESP32_IP>:{self.port}/callback')
        
        return s
    
    def handle_request(self, conn):
        request = conn.recv(1024).decode('utf-8')
        print('Request received:', request.split('\r\n')[0])
        
        # Parse request line
        request_line = request.split('\r\n')[0]
        method, path, _ = request_line.split(' ')
        
        if '/callback' in path and '?code=' in path:
            # Extract authorization code
            query = path.split('?')[1]
            params = dict(param.split('=') for param in query.split('&'))
            self.auth_code = params.get('code')
            
            response = self._success_response()
            conn.send(response)
            return True
        else:
            response = self._error_response()
            conn.send(response)
            return False
    
    def _success_response(self):
        html = """
        <html>
        <body>
        <h1>Authorization Successful!</h1>
        <p>You can close this window.</p>
        </body>
        </html>
        """
        return f"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n{html}".encode()
    
    def _error_response(self):
        return b"HTTP/1.1 404 Not Found\r\n\r\n"
    
    def wait_for_code(self, timeout=300):
        """Wait for OAuth callback with authorization code"""
        s = self.start()
        s.settimeout(timeout)
        
        try:
            while not self.auth_code:
                conn, addr = s.accept()
                print('Connection from:', addr)
                success = self.handle_request(conn)
                conn.close()
                if success:
                    break
        except OSError as e:
            print('Timeout or error:', e)
        finally:
            s.close()
        
        return self.auth_code

# Usage example
# server = SimpleHTTPServer(port=8080)
# code = server.wait_for_code()
# print('Authorization code:', code)
```

---

## Step 7: Strava OAuth Flow

Create `strava_auth.py`:

```python
# strava_auth.py
import urequests
import json

class StravaAuth:
    TOKEN_URL = 'https://www.strava.com/oauth/token'
    
    def __init__(self, client_id, client_secret):
        self.client_id = client_id
        self.client_secret = client_secret
        self.access_token = None
        self.refresh_token = None
    
    def exchange_code(self, code):
        """Exchange authorization code for access token"""
        payload = {
            'client_id': self.client_id,
            'client_secret': self.client_secret,
            'code': code,
            'grant_type': 'authorization_code'
        }
        
        try:
            response = urequests.post(self.TOKEN_URL, json=payload)
            data = response.json()
            response.close()
            
            if 'access_token' in data:
                self.access_token = data['access_token']
                self.refresh_token = data['refresh_token']
                print('Access token obtained!')
                return True
            else:
                print('Error:', data)
                return False
        except Exception as e:
            print('Request failed:', e)
            return False
    
    def refresh_access_token(self, refresh_token):
        """Refresh access token using refresh token"""
        payload = {
            'client_id': self.client_id,
            'client_secret': self.client_secret,
            'refresh_token': refresh_token,
            'grant_type': 'refresh_token'
        }
        
        try:
            response = urequests.post(self.TOKEN_URL, json=payload)
            data = response.json()
            response.close()
            
            if 'access_token' in data:
                self.access_token = data['access_token']
                return self.access_token
            else:
                print('Refresh error:', data)
                return None
        except Exception as e:
            print('Refresh failed:', e)
            return None
```

---

## Step 8: PostgreSQL Connectivity

MicroPython doesn't have native PostgreSQL drivers, but you have several options:

### Option A: HTTP API Bridge (Recommended)
Create a simple REST API on a server that communicates with PostgreSQL, then use `urequests` from ESP32.

**Server-side (Python Flask example):**
```python
# api_server.py (runs on your server, not ESP32)
from flask import Flask, request, jsonify
import psycopg2

app = Flask(__name__)

DB_CONFIG = {
    'host': 'your-db-host',
    'database': 'your-db',
    'user': 'your-user',
    'password': 'your-password'
}

@app.route('/activities', methods=['POST'])
def save_activity():
    data = request.json
    conn = psycopg2.connect(**DB_CONFIG)
    cur = conn.cursor()
    cur.execute(
        "INSERT INTO activities (user_id, activity_date, distance) VALUES (%s, %s, %s)",
        (data['user_id'], data['date'], data['distance'])
    )
    conn.commit()
    cur.close()
    conn.close()
    return jsonify({'status': 'success'})

@app.route('/activities/<user_id>', methods=['GET'])
def get_activities(user_id):
    conn = psycopg2.connect(**DB_CONFIG)
    cur = conn.cursor()
    cur.execute("SELECT * FROM activities WHERE user_id = %s", (user_id,))
    results = cur.fetchall()
    cur.close()
    conn.close()
    return jsonify({'activities': results})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
```

**ESP32 client (`db_client.py`):**
```python
# db_client.py
import urequests
import json

class DBClient:
    def __init__(self, api_url):
        self.api_url = api_url
    
    def save_activity(self, user_id, date, distance):
        payload = {
            'user_id': user_id,
            'date': date,
            'distance': distance
        }
        try:
            response = urequests.post(f'{self.api_url}/activities', json=payload)
            result = response.json()
            response.close()
            return result
        except Exception as e:
            print('DB save error:', e)
            return None
    
    def get_activities(self, user_id):
        try:
            response = urequests.get(f'{self.api_url}/activities/{user_id}')
            result = response.json()
            response.close()
            return result['activities']
        except Exception as e:
            print('DB fetch error:', e)
            return None

# Usage
# db = DBClient('http://your-server:5000')
# db.save_activity('user123', '2025-11-10', 5.2)
```

### Option B: micropg Library (Experimental)
There's an unofficial `micropg` library, but it's not well-maintained. Use with caution.

```python
# Not recommended for production, but possible:
# Upload micropg.py to ESP32
import micropg

conn = micropg.connect(
    host='your-host',
    user='your-user',
    password='your-pass',
    database='your-db'
)
cur = conn.cursor()
cur.execute('SELECT * FROM activities')
print(cur.fetchall())
conn.close()
```

**Recommendation**: Use Option A (HTTP API bridge) for reliability and ease of maintenance.

---

## Step 9: Project Structure

Organize your MicroPython project:

```
/esp32-strava/
├── boot.py              # WiFi setup, auto-runs on boot
├── main.py              # Main application logic
├── oauth_server.py      # HTTP server for OAuth callback
├── strava_auth.py       # Strava authentication
├── db_client.py         # Database API client
├── config.py            # Configuration (credentials)
└── lib/                 # External libraries (if any)
```

**Example `main.py`:**
```python
# main.py
from oauth_server import SimpleHTTPServer
from strava_auth import StravaAuth
from db_client import DBClient
import config

def main():
    print('Starting Strava OAuth flow...')
    
    # Step 1: Start OAuth server
    server = SimpleHTTPServer(port=8080)
    print('Waiting for OAuth callback...')
    code = server.wait_for_code()
    
    if not code:
        print('Failed to get authorization code')
        return
    
    # Step 2: Exchange code for token
    auth = StravaAuth(config.CLIENT_ID, config.CLIENT_SECRET)
    if auth.exchange_code(code):
        print('Access token:', auth.access_token)
        
        # Step 3: Save to database
        db = DBClient(config.API_URL)
        db.save_activity('user123', '2025-11-10', 5.2)
        print('Activity saved to database!')
    else:
        print('Token exchange failed')

if __name__ == '__main__':
    main()
```

**Example `config.py`:**
```python
# config.py
WIFI_SSID = 'YOUR_WIFI_SSID'
WIFI_PASSWORD = 'YOUR_WIFI_PASSWORD'

CLIENT_ID = 'your_strava_client_id'
CLIENT_SECRET = 'your_strava_client_secret'

API_URL = 'http://your-server:5000'
```

---

## Step 10: Upload Files to ESP32

### Option A: Using ampy
```bash
pip3 install adafruit-ampy

# Upload files
ampy --port /dev/ttyUSB0 put boot.py
ampy --port /dev/ttyUSB0 put main.py
ampy --port /dev/ttyUSB0 put oauth_server.py
ampy --port /dev/ttyUSB0 put strava_auth.py
ampy --port /dev/ttyUSB0 put db_client.py
ampy --port /dev/ttyUSB0 put config.py

# List files
ampy --port /dev/ttyUSB0 ls
```

### Option B: Using Thonny IDE
1. Open file in Thonny
2. File → Save As → MicroPython device
3. Choose filename

### Option C: Using mpremote (Official tool)
```bash
pip3 install mpremote

mpremote connect /dev/ttyUSB0 fs cp boot.py :boot.py
mpremote connect /dev/ttyUSB0 fs cp main.py :main.py
# ... repeat for other files
```

---

## Step 11: Testing

1. Reset ESP32 (press reset button or reconnect power)
2. Connect to REPL to see output
3. ESP32 should:
   - Connect to WiFi
   - Start OAuth server
   - Display IP address for redirect URI
4. In browser, navigate to Strava OAuth URL:
```
https://www.strava.com/oauth/authorize?client_id=YOUR_CLIENT_ID&response_type=code&redirect_uri=http://ESP32_IP:8080/callback&scope=activity:read_all
```
5. Authorize app → redirected to ESP32 → token exchange → save to database

---

## Troubleshooting

### WiFi Connection Issues
- Check SSID/password in `config.py`
- Ensure ESP32 is within range
- Try different WiFi channels/bands (2.4GHz only for most ESP32)

### Memory Errors
- ESP32 has limited RAM (~100KB free for MicroPython)
- Minimize concurrent connections
- Use `gc.collect()` to free memory
- Consider ESP32-S3 for more RAM

### urequests Not Found
Install micropython-lib packages:
```bash
mpremote connect /dev/ttyUSB0 mip install urequests
```

Or manually upload `urequests.py` from [micropython-lib](https://github.com/micropython/micropython-lib)

### Serial Port Permission Denied
```bash
sudo usermod -a -G dialout $USER
# Log out and back in
```

---

## Additional Resources

- [MicroPython Documentation](https://docs.micropython.org/en/latest/esp32/quickref.html)
- [ESP32 Pin Reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [Strava API Documentation](https://developers.strava.com/docs/reference/)
- [MicroPython Forum](https://forum.micropython.org/)

---

## Next Steps

1. Implement full Strava API integration (fetch activities)
2. Add error handling and retry logic
3. Implement token refresh mechanism
4. Add LED matrix display driver for calendar visualization
5. Set up periodic data sync
6. Implement local caching for offline operation

Good luck with your project!
