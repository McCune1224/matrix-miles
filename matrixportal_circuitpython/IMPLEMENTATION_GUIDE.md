# CircuitPython Implementation Guide

This is a comprehensive guide to understanding and extending the CircuitPython implementation for Matrix Miles.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    code.py (Main Loop)                      │
│  - Initializes components                                   │
│  - Manages update frequency                                 │
│  - Handles errors and logging                               │
└──────────────┬──────────────┬──────────────┬────────────────┘
               │              │              │
        ┌──────▼─────┐  ┌─────▼──────┐  ┌───▼─────────┐
        │   WiFi     │  │   API      │  │  Calendar   │
        │  Manager   │  │  Client    │  │  Display    │
        └────────────┘  └────────────┘  └─────────────┘
               │              │              │
        ┌──────▼──────────────▼──────────────▼─────┐
        │        CircuitPython Hardware Libs        │
        │  (wifi, displayio, adafruit_requests)    │
        └────────────────────────────────────────────┘
               │              │
        ┌──────▼──────┐  ┌────▼─────────┐
        │   WiFi      │  │  LED Matrix  │
        │  Hardware   │  │  Display     │
        └─────────────┘  └──────────────┘
```

## Component Breakdown

### 1. code.py - Main Application

**Purpose:** Entry point and main event loop

**Key Classes:**
- `MatrixMilesApp` - Main application controller

**Key Methods:**
- `__init__()` - Initialize all components
- `connect_wifi()` - Establish WiFi connection
- `fetch_activities()` - Get data from API
- `render_calendar()` - Display on matrix
- `update()` - Main update logic (called 1x/second)
- `run()` - Main event loop

**Flow:**
```
1. Initialize display, WiFi manager, API client
2. Connect to WiFi
3. Fetch initial activity data
4. Enter infinite loop:
   a. Check WiFi connection (every 1 second)
   b. Fetch new data if interval elapsed
   c. Render activities to display
   d. Sleep 1 second
```

**Configuration Injection:**
Uses `secrets.py` for configuration - never hardcoded.

### 2. lib/wifi_manager.py - WiFi Handling

**Purpose:** Manage WiFi connection and reconnection

**Key Class:**
- `WiFiManager` - Handles all WiFi operations

**Key Methods:**
- `connect(timeout)` - Connect with timeout
- `is_connected()` - Check current status
- `get_ip()` - Get IP address
- `get_signal_strength()` - Get RSSI
- `reconnect_if_needed()` - Auto-reconnect on interval

**Features:**
- Blocking connection with timeout
- Non-blocking reconnect attempts
- Signal strength monitoring
- Graceful error handling

**Usage:**
```python
from lib.wifi_manager import WiFiManager

wifi = WiFiManager(ssid="Network", password="pwd")
if wifi.connect(timeout=30):
    ip = wifi.get_ip()
    signal = wifi.get_signal_strength()
```

### 3. lib/api_client.py - API Communication

**Purpose:** Handle HTTPS requests to backend with SSL

**Key Class:**
- `APIClient` - HTTP/HTTPS client with error handling

**Key Methods:**
- `get_recent_activities()` - Fetch recent activities
- `get_calendar_data(year, month)` - Fetch calendar
- `get_user_stats()` - Fetch user statistics
- `health_check()` - Check API availability

**Features:**
- Automatic SSL/TLS certificate validation
- Built-in HTTP error handling
- JSON parsing
- Structured logging
- Timeout management

**Usage:**
```python
from lib.api_client import APIClient

client = APIClient(
    base_url="https://...",
    api_key="key",
    user_id=1,
    timeout=10
)

activities = client.get_recent_activities()
if activities:
    print(f"Got {len(activities)} activities")
```

**SSL/TLS Handling:**
```python
# Creates default SSL context with certificate validation
session = adafruit_requests.Session(
    pool, 
    ssl_context=ssl.create_default_context()
)
# Automatically validates certificates against trusted CAs
# No manual certificate fingerprint management needed!
```

### 4. lib/calendar_display.py - Display Rendering

**Purpose:** Render activity data on LED matrix

**Key Class:**
- `CalendarDisplay` - Display management and rendering

**Key Methods:**
- `clear()` - Clear display
- `show_message(text, duration)` - Show text message
- `render_calendar(activities)` - Render activities
- `set_brightness(value)` - Adjust brightness

**Features:**
- Display initialization
- Text rendering with fonts
- Activity grid layout
- Activity type summary
- Brightness control

**Current Implementation:**
Shows activity summary:
```
Activities: 5
run: 3
ride: 2
```

**Future Enhancements:**
- Calendar grid layout (7 columns for days)
- Color-coded activity types
- Heatmap view (darker = more activity)
- Weekly/monthly statistics
- Navigation between views

**Usage:**
```python
from lib.calendar_display import CalendarDisplay

display = CalendarDisplay(width=64, height=32)
display.show_message("Loading...")
# ... fetch data ...
display.render_calendar(activities)
```

## Data Flow

### Fetch Cycle

```
1. Check time elapsed since last fetch
2. If >= REFRESH_INTERVAL_SECONDS:
   a. Call api_client.get_recent_activities()
   b. Receive list of activity dicts
   c. Pass to calendar_display.render_calendar()
   d. Update display
   e. Record fetch time
```

### Activity Data Format

```python
activity = {
    "id": 1,
    "user_id": 1,
    "strava_activity_id": 12345,
    "name": "Morning Run",
    "activity_date": "2024-01-15",
    "type": "run",
    "distance": 5.2,
    "elapsed_time": 1800,
    "created_at": "2024-01-15T08:30:00Z"
}
```

## Configuration System

### Secrets Management

```python
# In code.py
from secrets import (
    WIFI_SSID,
    WIFI_PASSWORD,
    API_BASE_URL,
    API_KEY,
    USER_ID,
    REFRESH_INTERVAL_SECONDS
)
```

**Important:** `secrets.py` is in `.gitignore` and should never be committed.

### Environment Variables

Users configure by editing `secrets.py`:
```python
WIFI_SSID = "my-network"
WIFI_PASSWORD = "my-password"
API_BASE_URL = "https://matrix-miles-production.up.railway.app"
API_KEY = "abc123xyz"
USER_ID = 1
REFRESH_INTERVAL_SECONDS = 300
```

Device auto-reloads when file is saved.

## Error Handling

### Network Errors

**WiFi Disconnection:**
- Detected in main loop (every 1 second)
- Attempted reconnection after 10-second interval
- Graceful degradation (shows "No WiFi" message)

**API Request Failure:**
- Logged to serial with timestamp
- Error count incremented
- Retried on next fetch cycle
- Display shows "No activities" or previous data

### Application Errors

**Code Errors:**
- CircuitPython shows traceback on serial
- Auto-reloads code.py on save
- Can debug via REPL

**Library Errors:**
- Caught and logged
- Device continues running
- Check serial for details

## Logging & Debugging

### Log Levels

```
[timestamp] [LEVEL] message
```

**Levels:**
- `INFO` - Normal operation
- `WARNING` - Potential issues (e.g., WiFi reconnecting)
- `ERROR` - Problems (e.g., API request failed)

### Debug Output

Enable debug mode in code.py:
```python
DEBUG = True  # More output
DEBUG = False # Production (errors only)
```

### Serial Monitoring

```bash
# macOS/Linux
screen /dev/tty.usbmodem1 115200

# Windows (PowerShell)
$port = [System.IO.Ports.SerialPort]::getPortNames()[0]
Start-Process putty.exe -ArgumentList "-serial $port -sercfg 115200,8,n,1,X"
```

## Extension Points

### Adding New API Endpoints

1. Add method to `APIClient`:
```python
def get_weekly_stats(self, year, week):
    endpoint = f"/api/activities/weekly/{self.user_id}/{year}/{week}"
    return self._make_request(endpoint)
```

2. Call in main loop:
```python
stats = self.api_client.get_weekly_stats(2024, 4)
```

### Customizing Display

1. Edit `calendar_display.py`:
```python
def render_calendar(self, activities):
    # Custom rendering logic
    # Use displayio API for graphics
    # Use adafruit_display_text for text
```

2. Colors (RGB):
```python
WHITE = 0xFFFFFF
RED = 0xFF0000
GREEN = 0x00FF00
BLUE = 0x0000FF
YELLOW = 0xFFFF00
```

### Adding Activity Filtering

1. Create filter function:
```python
def filter_activities(activities, activity_type):
    return [a for a in activities if a['type'] == activity_type]
```

2. Use in render:
```python
run_activities = filter_activities(activities, 'run')
self.display.render_calendar(run_activities)
```

## Testing & Validation

### Manual Testing

```python
# In REPL
>>> from lib.api_client import APIClient
>>> client = APIClient("https://...", "key", 1)
>>> activities = client.get_recent_activities()
>>> len(activities)
3
>>> activities[0]['name']
'Morning Run'
```

### Integration Testing

1. Verify WiFi connects
2. Verify API responds
3. Verify display updates
4. Let run 24 hours
5. Check no error spikes

## Performance Characteristics

| Metric | Value |
|--------|-------|
| Startup time | ~1 second |
| WiFi connect | 2-5 seconds |
| API request | 2-3 seconds |
| Display render | ~100ms |
| Memory used | ~30KB code |
| Fetch interval | Configurable (300s default) |

## Security Considerations

### HTTPS/SSL

- Uses `ssl.create_default_context()`
- Validates certificates automatically
- No certificate pinning (flexible for cert rotation)
- Works with Railway's provided certificates

### API Key

- Stored in secrets.py (never in code)
- Sent via `X-API-Key` header
- No secrets logged to serial
- Should be rotated periodically

### WiFi

- Uses WPA2 authentication
- Password stored locally (not transmitted)
- Consider changing defaults after setup

## Deployment Considerations

### Power Management

Currently: Always on

Future options:
- Deep sleep between fetches
- Wake on timer
- Estimated 30-50% power reduction

### Network

- Requires stable WiFi (fallback to 2.4GHz)
- Reasonable for home/small office
- 5-minute intervals recommended

### Scalability

- Single device per MatrixPortal (current design)
- Can run multiple instances with different USER_IDs
- API rate-limiting not enforced (be respectful)

## Troubleshooting Guide

### Device won't boot

Check code.py syntax:
```python
>>> import code
# If error, syntax is wrong
```

### WiFi fails

Check in REPL:
```python
>>> import wifi
>>> wifi.radio.connect("SSID", "password")
>>> wifi.radio.connected
True
```

### API fails

Test endpoint:
```bash
curl -H "X-API-Key: key" https://api.example.com/api/activities/recent/1
```

### Display doesn't show

Check libraries installed:
```python
>>> import adafruit_display_text
>>> import board
>>> print(board.DISPLAY)
```

## Future Roadmap

1. **Enhanced Display:**
   - Calendar grid layout
   - Color-coded activity types
   - Weekly/monthly views
   - Touch controls

2. **Smart Features:**
   - Automatic refresh rate adjustment
   - Activity notifications
   - Streak tracking
   - Goal progress display

3. **Power Optimization:**
   - Deep sleep between fetches
   - Wake-on-event support
   - Scheduled display on/off

4. **Multi-Device:**
   - Device discovery
   - Config sync
   - Activity filtering by type

## Resources

- [CircuitPython Docs](https://circuitpython.readthedocs.io/)
- [MatrixPortal M4 Guide](https://learn.adafruit.com/adafruit-matrixportal-m4/)
- [Adafruit Libraries](https://github.com/adafruit/circuitpython_bundle)
- [HTTP Requests](https://circuitpython.readthedocs.io/projects/requests/en/latest/)
- [Display API](https://circuitpython.readthedocs.io/en/latest/shared-bindings/displayio/index.html)

## Contributing

To extend or improve:

1. Follow existing code style
2. Add comprehensive comments
3. Test thoroughly on device
4. Document changes
5. Update this guide

---

**Last Updated:** November 2024
**Status:** Production Ready
**Maintainer:** Your Project
