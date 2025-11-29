# MatrixPortal M4 - Getting Started with CircuitPython

This guide walks you through setting up CircuitPython on your MatrixPortal M4 and configuring it to fetch Strava activities from your Matrix Miles backend.

**Time estimate: 30-45 minutes**

## Prerequisites

- MatrixPortal M4 board
- USB cable (USB-C)
- Computer with USB port
- WiFi network (2.4GHz, not 5GHz)
- Your Matrix Miles backend running and accessible
- API key from your backend

## Step 1: Install CircuitPython

### 1.1 Download CircuitPython

1. Go to https://circuitpython.org/board/matrixportal_m4/
2. Download the latest **stable** release (8.2.x or later)
3. You'll get a `.uf2` file

### 1.2 Put Device in Bootloader Mode

1. Connect MatrixPortal M4 to USB
2. Look for a small reset button on the board (near the USB connector)
3. **Double-click the reset button** quickly
4. The board will appear as a USB drive named `MATRIXPORTAL`

### 1.3 Upload CircuitPython

1. Drag and drop the `.uf2` file onto the `MATRIXPORTAL` drive
2. The board will reboot automatically
3. After reboot, a new drive named `CIRCUITPY` will appear

**✓ CircuitPython is now installed!**

## Step 2: Install Required Libraries

CircuitPython uses a library bundle. We need to add specific libraries to your device.

### 2.1 Download the Library Bundle

1. Go to https://circuitpython.org/libraries
2. Download the CircuitPython Library Bundle for your version (match CircuitPython version)
3. Extract the ZIP file

### 2.2 Install Required Libraries

Copy these folders from the extracted bundle to `CIRCUITPY/lib/`:

**Required:**
- `adafruit_requests.mpy` - HTTP requests with SSL/TLS
- `adafruit_display_text/` - Text rendering on display
- `adafruit_bitmap_font/` - Font support

**Optional but recommended:**
- `adafruit_bus_device/` - I2C/SPI communication
- `neopixel.mpy` - NeoPixel LED control (if using RGB LEDs)

```bash
# Example for macOS/Linux
cp /path/to/bundle/adafruit_requests.mpy /Volumes/CIRCUITPY/lib/
cp -r /path/to/bundle/adafruit_display_text /Volumes/CIRCUITPY/lib/
cp -r /path/to/bundle/adafruit_bitmap_font /Volumes/CIRCUITPY/lib/
```

### 2.3 Verify Installation

The `/Volumes/CIRCUITPY/lib/` folder should now contain:
```
lib/
├── adafruit_requests.mpy
├── adafruit_display_text/
├── adafruit_bitmap_font/
└── ... (other libraries)
```

## Step 3: Configure Your Settings

### 3.1 Create secrets.py

1. Copy `config.example.py` to `secrets.py`
2. Edit `secrets.py` with your configuration:

```python
# WiFi Configuration
WIFI_SSID = "your-actual-wifi-name"
WIFI_PASSWORD = "your-actual-password"

# API Configuration
API_BASE_URL = "https://matrix-miles-production.up.railway.app"
API_KEY = "your-api-key-from-backend"
USER_ID = 1

# Display Configuration
REFRESH_INTERVAL_SECONDS = 300  # 5 minutes
DISPLAY_BRIGHTNESS = 1.0
```

### 3.2 Upload to Device

```bash
# Copy config to device (macOS/Linux)
cp config.example.py /Volumes/CIRCUITPY/secrets.py
nano /Volumes/CIRCUITPY/secrets.py  # Edit the values
```

**Important:** `secrets.py` contains credentials and is in `.gitignore`. Never commit it to git!

## Step 4: Upload Application Code

Copy the application files to your device:

```bash
# Copy main application
cp code.py /Volumes/CIRCUITPY/

# Copy library modules
cp lib/wifi_manager.py /Volumes/CIRCUITPY/lib/
cp lib/api_client.py /Volumes/CIRCUITPY/lib/
cp lib/calendar_display.py /Volumes/CIRCUITPY/lib/
```

Your device should now have:
```
CIRCUITPY/
├── code.py
├── secrets.py
├── lib/
│   ├── wifi_manager.py
│   ├── api_client.py
│   ├── calendar_display.py
│   ├── adafruit_requests.mpy
│   ├── adafruit_display_text/
│   └── adafruit_bitmap_font/
└── ... (other library bundles)
```

## Step 5: Monitor and Debug

### 5.1 Open Serial Monitor

Connect a serial monitor to see debug output:

**Using `screen` (macOS/Linux):**
```bash
screen /dev/tty.usbmodem1 115200
# Press Ctrl+A then Ctrl+X to exit
```

**Using Arduino IDE:**
1. Tools → Serial Monitor
2. Select your device port
3. Set baud rate to 115200

### 5.2 Expected Output

When the device boots, you should see:

```
[0.1] [INFO] === Matrix Miles (CircuitPython) ===
[0.2] [INFO] Display initialized
[0.3] [INFO] WiFi manager created
[0.5] [INFO] API client created
[1.2] [INFO] Connecting to WiFi...
[3.5] [INFO] WiFi connected: 192.168.1.100
[5.0] [INFO] Fetching activities...
[6.2] [INFO] API response received: 3 activities
[6.3] [INFO] Rendering 3 activities
[6.4] [INFO] Calendar rendered successfully
[6.5] [INFO] Status: fetches=1, errors=0
```

## Step 6: Troubleshooting

### WiFi Connection Fails

**Symptom:** `[ERROR] WiFi connection failed`

**Solutions:**
1. Check WiFi credentials in `secrets.py`
2. Ensure 2.4GHz network (MatrixPortal M4 doesn't support 5GHz)
3. Check signal strength near the device
4. Restart the device (press reset button)

```bash
# To check your WiFi settings
cat /Volumes/CIRCUITPY/secrets.py
```

### API Request Fails

**Symptom:** `[ERROR] API request error: ...`

**Solutions:**
1. Verify `API_BASE_URL` is correct
   - Production: `https://matrix-miles-production.up.railway.app`
   - Local test: `http://192.168.1.100:8080`
2. Check API key is correct:
   ```bash
   # Get API key from backend logs
   ```
3. Test API manually:
   ```bash
   curl -H "X-API-Key: your-key" \
        https://matrix-miles-production.up.railway.app/api/activities/recent/1
   ```

### SSL/Certificate Errors

**Symptom:** `[ERROR] SSL: CERTIFICATE_VERIFY_FAILED`

**Solutions:**
1. Ensure you're using HTTPS with a valid certificate
2. For self-signed certificates (dev only):
   - Modify `api_client.py` line 36 to skip verification (NOT for production!)

### No Activities Display

**Symptom:** Device shows "No activities"

**Solutions:**
1. Check user ID matches actual data:
   ```bash
   curl -H "X-API-Key: your-key" \
        https://matrix-miles-production.up.railway.app/api/activities/recent/1
   ```
2. Ensure activities exist for the specified user
3. Check date format in backend response (should be YYYY-MM-DD)

### Device Doesn't Boot

**Symptom:** Device goes dark after plugging in, or shows error on serial monitor

**Solutions:**
1. Check `code.py` for syntax errors
2. Open REPL and test imports:
   ```python
   >>> import wifi
   >>> print(wifi.radio)
   ```
3. Reset device to safe mode:
   - Hold down the reset button on the board while power is connected
   - Release after the built-in LED changes color

## Step 7: Testing API Connectivity

Before deploying, test your API connectivity:

### 7.1 Test with curl

```bash
# Get recent activities
curl -H "X-API-Key: your-api-key" \
     https://matrix-miles-production.up.railway.app/api/activities/recent/1 | jq

# Expected response:
# [
#   {
#     "id": 1,
#     "user_id": 1,
#     "name": "Morning Run",
#     "activity_date": "2024-01-15",
#     "type": "run",
#     "distance": 5.2,
#     ...
#   }
# ]
```

### 7.2 Manual REPL Testing

Connect to serial and test in REPL:

```python
# Enter REPL (Ctrl+C to stop current code)
>>> import wifi
>>> from lib.api_client import APIClient
>>> 
>>> client = APIClient(
...     base_url="https://...",
...     api_key="your-key",
...     user_id=1
... )
>>> activities = client.get_recent_activities()
>>> print(activities)
```

## Step 8: Customization

### Change Refresh Interval

Edit `secrets.py`:
```python
REFRESH_INTERVAL_SECONDS = 60  # Check every minute instead of 5
```

### Change Display Brightness

Edit `secrets.py`:
```python
DISPLAY_BRIGHTNESS = 0.5  # 50% brightness
```

### Add Custom Display Logic

Edit `lib/calendar_display.py`:
- Modify `render_calendar()` to customize layout
- Add color coding for activity types
- Implement activity filtering

## Step 9: Power Management (Optional)

For long-term deployment:

1. Use a power bank with auto-off timer
2. Implement deep sleep mode (see CircuitPython docs)
3. Monitor power consumption with a USB meter
4. Set reasonable refresh intervals (300+ seconds)

## Next Steps

1. **Basic operation:** Verify activities display for 24 hours
2. **Customization:** Adjust colors, layout, and refresh interval
3. **Advanced:** Add button controls, activity filtering, or statistics display
4. **Production:** Deploy to permanent location with stable power

## Getting Help

- CircuitPython docs: https://circuitpython.readthedocs.io/
- MatrixPortal M4 guide: https://learn.adafruit.com/adafruit-matrixportal-m4/overview
- Adafruit libraries: https://github.com/adafruit/circuitpython_bundle
- Matrix Miles issues: Check the project repository

## File Structure Reference

```
matrixportal_circuitpython/
├── README.md                 # Project overview
├── GETTING_STARTED.md       # This file
├── code.py                  # Main application (copy to CIRCUITPY/)
├── config.example.py        # Configuration template (copy as secrets.py)
├── lib/
│   ├── wifi_manager.py      # WiFi connection logic
│   ├── api_client.py        # API communication
│   └── calendar_display.py  # Display rendering
└── docs/
    └── API_ENDPOINTS.md     # Backend API reference
```

---

**Ready?** Start with Step 1 and work through each section. Good luck! 🚀
