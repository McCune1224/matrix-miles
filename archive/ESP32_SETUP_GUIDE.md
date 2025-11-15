# ESP32 Setup Guide - Next Session Workflow

## Prerequisites Checklist

Before starting, ensure you have:
- [ ] ESP32 board (any ESP32 variant with WiFi)
- [ ] USB cable for ESP32
- [ ] Python 3.7+ installed on your machine
- [ ] Go server running and accessible (confirmed working in previous session)
- [ ] Your User ID from the OAuth flow

## Step 1: Prepare Your Environment (5 minutes)

### Install Required Tools

```bash
# Install esptool for flashing firmware
pip3 install esptool

# Install mpremote for file management
pip3 install mpremote

# Alternative: Install ampy (if mpremote doesn't work)
pip3 install adafruit-ampy
```

### Find Your ESP32 Port

```bash
# Linux/Mac - list serial devices
ls /dev/tty* | grep -i usb

# Common ports:
# Linux: /dev/ttyUSB0 or /dev/ttyACM0
# Mac: /dev/tty.usbserial-* or /dev/tty.SLAB_USBtoUART
# Windows: COM3, COM4, etc.

# Test connection
ls -la /dev/ttyUSB0  # Replace with your port
```

## Step 2: Flash MicroPython Firmware (10 minutes)

### Download Firmware

```bash
cd ~/Downloads

# Download latest stable MicroPython for ESP32
wget https://micropython.org/resources/firmware/ESP32_GENERIC-20240602-v1.23.0.bin

# Or use curl if wget not available
curl -O https://micropython.org/resources/firmware/ESP32_GENERIC-20240602-v1.23.0.bin
```

### Flash the Firmware

```bash
# Step 1: Erase existing flash (IMPORTANT!)
esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash

# Step 2: Flash MicroPython
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x1000 ESP32_GENERIC-20240602-v1.23.0.bin
```

**Expected Output:**
```
Hard resetting via RTS pin...
```

### Verify Flash Success

```bash
# Connect to REPL
mpremote connect /dev/ttyUSB0

# Or using screen
screen /dev/ttyUSB0 115200

# You should see Python REPL prompt:
# >>>
```

Press `Ctrl+D` to reboot and `Ctrl+C` to interrupt if needed.

## Step 3: Create ESP32 Project Files (10 minutes)

### Create Project Directory

```bash
cd ~/Code/micro-srava
mkdir -p esp32-client
cd esp32-client
```

### Create Configuration File

Create `config.py`:

```python
# config.py - ESP32 Configuration

# WiFi credentials
WIFI_SSID = 'YOUR_WIFI_NAME'
WIFI_PASSWORD = 'YOUR_WIFI_PASSWORD'

# Server configuration
# Use your Railway server URL or ngrok tunnel for testing
# For local testing: 'http://192.168.1.XXX:8080' (your machine's IP)
API_BASE_URL = 'http://localhost:8080'  # UPDATE THIS!

# Your ESP32 API key (from strava-server/.env)
API_KEY = '9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6'

# Your Strava user ID (get this from OAuth flow)
USER_ID = 1  # UPDATE THIS after OAuth!

# How often to refresh data (seconds)
REFRESH_INTERVAL = 300  # 5 minutes
```

**IMPORTANT NOTES:**
- Replace `WIFI_SSID` and `WIFI_PASSWORD` with your actual WiFi
- If testing locally, update `API_BASE_URL` to your machine's IP (see Step 4)
- The `USER_ID` comes from completing the OAuth flow in your browser
- The `API_KEY` is already set from your `.env` file

### Create Boot Script

Create `boot.py`:

```python
# boot.py - Runs on ESP32 startup

import network
import time
from config import WIFI_SSID, WIFI_PASSWORD

def connect_wifi():
    """Connect to WiFi network"""
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    
    if wlan.isconnected():
        print('Already connected to WiFi')
        print('IP:', wlan.ifconfig()[0])
        return True
    
    print('Connecting to WiFi:', WIFI_SSID)
    wlan.connect(WIFI_SSID, WIFI_PASSWORD)
    
    # Wait up to 15 seconds for connection
    timeout = 15
    start = time.time()
    
    while not wlan.isconnected():
        if time.time() - start > timeout:
            print('ERROR: WiFi connection timeout!')
            print('Check SSID and password in config.py')
            return False
        
        print('.', end='')
        time.sleep(0.5)
    
    print('\n✓ WiFi connected!')
    print('IP address:', wlan.ifconfig()[0])
    print('Gateway:', wlan.ifconfig()[2])
    return True

# Auto-connect on boot
print('=== ESP32 Strava Client Starting ===')
if connect_wifi():
    print('Ready to run main.py')
else:
    print('WiFi connection failed!')
```

### Create API Client

Create `api_client.py`:

```python
# api_client.py - HTTP client for Strava server

try:
    import urequests as requests
except ImportError:
    import requests  # Fallback for testing

import json
from config import API_BASE_URL, API_KEY, USER_ID

class StravaClient:
    """Client for communicating with Go server"""
    
    def __init__(self):
        self.base_url = API_BASE_URL.rstrip('/')
        self.user_id = USER_ID
        self.headers = {
            'X-API-Key': API_KEY,
            'Content-Type': 'application/json'
        }
    
    def health_check(self):
        """Check if server is reachable"""
        try:
            url = f'{self.base_url}/health'
            print(f'Checking: {url}')
            response = requests.get(url, timeout=5)
            success = response.status_code == 200
            response.close()
            return success
        except Exception as e:
            print(f'Health check failed: {e}')
            return False
    
    def get_recent_activities(self, limit=5):
        """Fetch recent activities from server"""
        try:
            url = f'{self.base_url}/api/activities/recent/{self.user_id}'
            print(f'Fetching: {url}')
            
            response = requests.get(url, headers=self.headers, timeout=10)
            
            if response.status_code == 200:
                data = response.json()
                response.close()
                return data[:limit] if data else []
            elif response.status_code == 401:
                print('ERROR: Invalid API key')
                response.close()
                return None
            else:
                print(f'ERROR: Server returned {response.status_code}')
                response.close()
                return None
                
        except Exception as e:
            print(f'Request failed: {e}')
            return None
    
    def get_stats(self):
        """Fetch user statistics"""
        try:
            url = f'{self.base_url}/api/stats/{self.user_id}'
            print(f'Fetching: {url}')
            
            response = requests.get(url, headers=self.headers, timeout=10)
            
            if response.status_code == 200:
                data = response.json()
                response.close()
                return data
            else:
                print(f'ERROR: Server returned {response.status_code}')
                response.close()
                return None
                
        except Exception as e:
            print(f'Request failed: {e}')
            return None
    
    def get_calendar(self, year, month):
        """Fetch calendar data for a specific month"""
        try:
            url = f'{self.base_url}/api/activities/calendar/{self.user_id}/{year}/{month}'
            print(f'Fetching: {url}')
            
            response = requests.get(url, headers=self.headers, timeout=10)
            
            if response.status_code == 200:
                data = response.json()
                response.close()
                return data
            else:
                print(f'ERROR: Server returned {response.status_code}')
                response.close()
                return None
                
        except Exception as e:
            print(f'Request failed: {e}')
            return None
```

### Create Main Application

Create `main.py`:

```python
# main.py - Main application logic

import time
import gc
from api_client import StravaClient
from config import REFRESH_INTERVAL

def format_distance(meters):
    """Convert meters to km with 2 decimal places"""
    return f'{meters / 1000:.2f}'

def format_duration(seconds):
    """Convert seconds to hours:minutes"""
    hours = seconds // 3600
    minutes = (seconds % 3600) // 60
    return f'{hours}h {minutes}m'

def display_activities(activities):
    """Display recent activities in console"""
    if not activities:
        print('No activities found')
        return
    
    print('\n' + '='*50)
    print('RECENT ACTIVITIES')
    print('='*50)
    
    for i, activity in enumerate(activities, 1):
        name = activity.get('name', 'Unknown')
        activity_type = activity.get('type', 'Unknown')
        distance_km = format_distance(activity.get('distance', 0))
        duration = format_duration(activity.get('moving_time', 0))
        date = activity.get('start_date', '')[:10]
        
        print(f'{i}. {name}')
        print(f'   Type: {activity_type} | Date: {date}')
        print(f'   Distance: {distance_km} km | Duration: {duration}')
        print()

def display_stats(stats):
    """Display user statistics"""
    if not stats:
        print('No stats available')
        return
    
    total_activities = stats.get('total_activities', 0)
    total_distance = format_distance(stats.get('total_distance', 0))
    total_time = format_duration(stats.get('total_time', 0))
    
    print('\n' + '='*50)
    print('YOUR STATS')
    print('='*50)
    print(f'Total Activities: {total_activities}')
    print(f'Total Distance: {total_distance} km')
    print(f'Total Time: {total_time}')
    print('='*50)

def main():
    """Main application loop"""
    print('\n' + '='*50)
    print('ESP32 Strava Activity Display')
    print('='*50)
    
    # Initialize client
    client = StravaClient()
    
    # Test server connection
    print('\nTesting server connection...')
    if not client.health_check():
        print('\nERROR: Cannot reach server!')
        print('Please check:')
        print('1. API_BASE_URL in config.py is correct')
        print('2. Go server is running')
        print('3. ESP32 can reach the server (firewall/network)')
        return
    
    print('✓ Server connection successful!')
    
    # Main loop
    iteration = 0
    while True:
        try:
            iteration += 1
            print(f'\n{"="*50}')
            print(f'Update #{iteration} - {time.localtime()}')
            print('='*50)
            
            # Fetch and display recent activities
            print('\nFetching recent activities...')
            activities = client.get_recent_activities(limit=5)
            if activities:
                display_activities(activities)
            else:
                print('Failed to fetch activities')
            
            # Fetch and display stats
            print('\nFetching stats...')
            stats = client.get_stats()
            if stats:
                display_stats(stats)
            else:
                print('Failed to fetch stats')
            
            # TODO: Add LED matrix display here
            # display_on_matrix(activities)
            
            # Free memory
            gc.collect()
            free_mem = gc.mem_free()
            print(f'\nFree memory: {free_mem} bytes')
            print(f'Next update in {REFRESH_INTERVAL} seconds...')
            
            time.sleep(REFRESH_INTERVAL)
            
        except KeyboardInterrupt:
            print('\n\nShutting down...')
            print('Press Ctrl+C again to exit to REPL')
            break
            
        except Exception as e:
            print(f'\nERROR in main loop: {e}')
            print('Waiting 30 seconds before retry...')
            time.sleep(30)

# Run main if executed directly
if __name__ == '__main__':
    main()
```

## Step 4: Get Your User ID (Required!)

Before uploading to ESP32, you need your User ID:

```bash
# Option 1: Complete OAuth flow in browser
# 1. Make sure your server is running:
cd ~/Code/micro-srava/strava-server
./server

# 2. Visit in browser:
# http://localhost:8080/auth/login

# 3. After authorizing, note the User ID shown on success page

# Option 2: Query database directly
psql "postgresql://postgres:nuzXwPdIMuEkqSWiMlFDDrJBzuXWDxoh@hopper.proxy.rlwy.net:37026/railway" -c "SELECT id, strava_user_id FROM users;"
```

**Update `config.py` with your User ID before uploading!**

## Step 5: Configure Server Access

### Option A: Local Network (Easiest for testing)

Find your computer's IP address:

```bash
# Linux/Mac
ip addr show | grep inet
# or
ifconfig | grep inet

# Look for something like: 192.168.1.XXX or 10.0.0.XXX
```

Update `config.py`:
```python
API_BASE_URL = 'http://192.168.1.XXX:8080'  # Your machine's IP
```

Make sure your Go server is running and accessible from your network.

### Option B: Use ngrok for Testing (If local network doesn't work)

```bash
# Install ngrok
# Download from https://ngrok.com/download

# Start ngrok tunnel
ngrok http 8080

# Copy the https URL (e.g., https://abc123.ngrok.io)
# Update config.py:
# API_BASE_URL = 'https://abc123.ngrok.io'
```

### Option C: Railway Production URL (Best for long-term)

If your server is deployed to Railway, use that URL directly:

```python
API_BASE_URL = 'https://your-app.railway.app'
```

## Step 6: Upload Files to ESP32 (5 minutes)

### Upload All Files

```bash
cd ~/Code/micro-srava/esp32-client

# Install urequests library on ESP32
mpremote connect /dev/ttyUSB0 mip install urequests

# Upload project files
mpremote connect /dev/ttyUSB0 fs cp config.py :config.py
mpremote connect /dev/ttyUSB0 fs cp boot.py :boot.py
mpremote connect /dev/ttyUSB0 fs cp api_client.py :api_client.py
mpremote connect /dev/ttyUSB0 fs cp main.py :main.py

# Verify files were uploaded
mpremote connect /dev/ttyUSB0 fs ls
```

Expected output:
```
ls :
         119 boot.py
         450 config.py
        2341 api_client.py
        3012 main.py
```

## Step 7: Test ESP32 Application (5 minutes)

### Connect to REPL

```bash
mpremote connect /dev/ttyUSB0
```

### Run the Application

```python
>>> import main
```

### Expected Output

```
=== ESP32 Strava Client Starting ===
Connecting to WiFi: YourWiFiName
.....
✓ WiFi connected!
IP address: 192.168.1.100

==================================================
ESP32 Strava Activity Display
==================================================

Testing server connection...
Checking: http://192.168.1.XXX:8080/health
✓ Server connection successful!

==================================================
Update #1 - (2025, 11, 10, 14, 30, 0, 6, 314)
==================================================

Fetching recent activities...
Fetching: http://192.168.1.XXX:8080/api/activities/recent/1

==================================================
RECENT ACTIVITIES
==================================================
1. Morning Run
   Type: Run | Date: 2025-11-09
   Distance: 5.23 km | Duration: 0h 32m

...
```

## Step 8: Make It Auto-Start (Optional)

If you want the application to run automatically on boot:

```python
# Connect to REPL
mpremote connect /dev/ttyUSB0

# Edit boot.py to include main.py import at the end
# Add this line at the bottom of boot.py:
>>> import main
```

Or keep it manual and just press Ctrl+D to reboot and run main manually.

## Troubleshooting Guide

### ESP32 Won't Connect to WiFi

```python
# Test WiFi manually
>>> import network
>>> wlan = network.WLAN(network.STA_IF)
>>> wlan.active(True)
>>> wlan.scan()  # List available networks
>>> wlan.connect('SSID', 'PASSWORD')
>>> wlan.isconnected()
>>> wlan.ifconfig()  # Shows IP if connected
```

### Can't Reach Server

```python
# Test basic connectivity
>>> import socket
>>> addr = socket.getaddrinfo('google.com', 80)[0][-1]
>>> s = socket.socket()
>>> s.connect(addr)  # Should work if internet is OK
>>> s.close()

# Test server directly
>>> import urequests
>>> response = urequests.get('http://YOUR_SERVER_IP:8080/health')
>>> print(response.status_code)  # Should be 200
>>> response.close()
```

### API Key Issues

```python
# Verify API key
>>> from config import API_KEY
>>> print(API_KEY)  # Should match strava-server/.env

# Test with curl from your computer
curl -H "X-API-Key: YOUR_KEY" http://localhost:8080/api/stats/1
```

### Memory Issues

```python
>>> import gc
>>> gc.collect()
>>> print(gc.mem_free())  # Should have >50000 bytes free

# If low memory, reduce REFRESH_INTERVAL or fetch less data
```

### File Upload Issues

```bash
# If mpremote doesn't work, try ampy
pip3 install adafruit-ampy

ampy --port /dev/ttyUSB0 put config.py
ampy --port /dev/ttyUSB0 put boot.py
ampy --port /dev/ttyUSB0 put api_client.py
ampy --port /dev/ttyUSB0 put main.py

# List files
ampy --port /dev/ttyUSB0 ls
```

## Next Steps After Testing

Once the basic setup works:

1. **Add LED Matrix Display**
   - Get MAX7219 LED matrix module
   - Install max7219 MicroPython library
   - Update main.py to display activities on matrix

2. **Improve Error Handling**
   - Add retry logic for failed requests
   - Better WiFi reconnection
   - Watchdog timer for crashes

3. **Add Features**
   - Display different activity types with different patterns
   - Show monthly calendar on matrix
   - Add button to cycle through different views
   - Battery power optimization (deep sleep between updates)

4. **Deploy Production Server**
   - Move from localhost to Railway/cloud
   - Update ESP32 config with production URL
   - Set up SSL/TLS for security

## Quick Reference Commands

```bash
# Flash firmware
esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x1000 firmware.bin

# Upload files
mpremote connect /dev/ttyUSB0 fs cp file.py :file.py

# Connect to REPL
mpremote connect /dev/ttyUSB0
# Or
screen /dev/ttyUSB0 115200

# Run program
>>> import main

# Soft reboot
>>> import machine
>>> machine.soft_reset()

# Hard reset
>>> machine.reset()
```

## Summary Checklist

Before your next session, make sure you have:
- [ ] ESP32 board and USB cable
- [ ] Python tools installed (esptool, mpremote)
- [ ] Go server running and tested
- [ ] User ID from OAuth flow
- [ ] WiFi credentials ready
- [ ] Server URL/IP address configured
- [ ] This guide saved for reference

Good luck with your ESP32 setup! The code is designed to be simple and verbose with helpful error messages to guide you through any issues.
