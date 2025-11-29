# Make Commands Guide

Complete reference for `make` commands available in the CircuitPython project.

## Overview

The `Makefile` provides convenient commands for development, debugging, and deployment to your MatrixPortal M4 device.

## Quick Start

```bash
# First time setup
make config              # Create secrets.py from template

# Development cycle
make upload              # Upload code changes to device
make serial              # Watch real-time output
```

## Available Commands

### Monitoring & Debugging

#### `make serial` / `make monitor`
View real-time serial output from your device (115200 baud).

**Usage:**
```bash
make serial
```

**Exit:** Press `Ctrl+A` then `Ctrl+X` (if using `screen`)

**Tools:** Automatically uses available tool (`screen`, `picocom`, or `minicom`)

**What you'll see:**
- WiFi connection status
- API request logs
- Activity data
- Any errors or warnings

**Example output:**
```
[0.1] [INFO] === Matrix Miles (CircuitPython) ===
[1.2] [INFO] Connecting to WiFi...
[3.5] [INFO] WiFi connected: 192.168.1.100
[5.0] [INFO] Fetching activities...
[6.2] [INFO] API response received: 5 activities
```

#### `make repl`
Enter interactive REPL for live debugging.

**Usage:**
```bash
make repl
```

**Commands in REPL:**
```python
>>> import wifi
>>> wifi.radio.connected
True

>>> from lib.api_client import APIClient
>>> client = APIClient("https://...", "key", 1)
>>> activities = client.get_recent_activities()
>>> len(activities)
5

>>> print(activities[0]['name'])
Morning Run
```

**Exit:** Type `exit()`

**Keyboard shortcuts:**
- `Ctrl+C` - Stop running code
- `Ctrl+D` - Soft reset
- `Ctrl+E` - Enter paste mode
- `Ctrl+A` - Go to beginning of line

#### `make debug`
Show debug mode configuration and enable verbose logging.

**Usage:**
```bash
make debug              # Show current DEBUG setting
```

**To enable debug mode:**
1. Edit `code.py`
2. Find `DEBUG = False` near the top
3. Change to `DEBUG = True`
4. Save file
5. Run: `make upload && make serial`

**Debug output includes:**
- Detailed function calls
- Variable values
- Extra status messages

### File Management

#### `make upload` / `make sync` / `make copy`
Upload `code.py` and `lib/` directory to device.

**Usage:**
```bash
make upload              # Upload all files
```

**What happens:**
1. Verifies `code.py` exists
2. Verifies `lib/` directory exists
3. Copies `code.py` to CIRCUITPY/
4. Copies all files in `lib/` to CIRCUITPY/lib/
5. Device auto-reloads in a few seconds

**Device mount point detection:**
- macOS: `/Volumes/CIRCUITPY`
- Linux: `/media/$USER/CIRCUITPY`

**Example workflow:**
```bash
# Edit code.py
vim code.py

# Upload changes
make upload

# Watch output
make serial
```

#### `make config`
Setup configuration file from template.

**Usage:**
```bash
make config              # Create secrets.py
```

**What happens:**
1. Copies `config.example.py` to `secrets.py`
2. Backs up existing `secrets.py` (if present)
3. Provides instructions for next steps

**After running:**
1. Edit `secrets.py` with your settings:
   ```python
   WIFI_SSID = "your-network"
   WIFI_PASSWORD = "password"
   API_BASE_URL = "https://..."
   API_KEY = "your-key"
   USER_ID = 1
   ```
2. Run: `make upload`
3. Run: `make serial`

### Verification & Testing

#### `make status`
Check device and mount status.

**Usage:**
```bash
make status
```

**Shows:**
- CIRCUITPY mount point
- Serial port location
- Files on device
- Local file status
- Configuration status

**Example output:**
```
Device Status Check
═══════════════════════════════════════════════════════════

Mount Point:
  ✓ CIRCUITPY mounted at: /Volumes/CIRCUITPY
  Files on device:
    boot.py
    code.py
    lib/
    secrets.py

Serial Port:
  ✓ Device found at: /dev/tty.usbmodem1

Local Files:
  code.py: ✓
  lib/: ✓
  config.example.py: ✓
  secrets.py: ✓
```

#### `make check`
Verify file structure on device.

**Usage:**
```bash
make check               # Verify all files present
```

**Checks:**
- ✓ `code.py` (and line count)
- ✓ `lib/api_client.py`
- ✓ `lib/wifi_manager.py`
- ✓ `lib/calendar_display.py`
- ✓ `adafruit_requests.mpy`
- ✓ `adafruit_display_text/`
- ✓ `adafruit_bitmap_font/`

**Example:**
```bash
$ make check
Verifying device file structure...

Expected files:
  ✓ code.py (165 lines)
  ✓ lib/api_client.py (180 lines)
  ✓ lib/wifi_manager.py (115 lines)
  ✓ lib/calendar_display.py (170 lines)

Required Adafruit libraries:
  ✓ adafruit_requests.mpy
  ✓ adafruit_display_text/
  ✓ adafruit_bitmap_font/
```

#### `make test`
Test API connectivity.

**Usage:**
```bash
make test                # Test health endpoint
```

**With API key:**
```bash
make test-api API_KEY=your-key USER_ID=1
```

**What happens:**
1. Tests `/health` endpoint
2. Fetches activities from API
3. Pretty-prints JSON response

**Example:**
```bash
$ make test-api API_KEY=abc123 USER_ID=1
Testing API request...
Base URL: https://matrix-miles-production.up.railway.app
User ID: 1

Fetching activities...
[
  {
    "id": 1,
    "name": "Morning Run",
    "activity_date": "2024-01-15",
    "type": "run",
    "distance": 5.2
  }
]
```

### Utilities

#### `make clean`
Remove local temporary files.

**Usage:**
```bash
make clean               # Remove .pyc, __pycache__, etc.
```

**Removes:**
- `*.pyc` files
- `__pycache__/` directories
- `*.swp` files (vim)
- `*~` backup files

#### `make help`
Show all available commands with descriptions.

**Usage:**
```bash
make help                # Print this help
```

**Color-coded output:**
- Blue: Headings
- Green: Command names
- Yellow: Examples and notes

## Common Workflows

### First-Time Setup

```bash
# Create configuration
make config

# Edit secrets.py with your settings
# (Open the file and enter WiFi/API details)

# Upload to device
make upload

# Verify files are on device
make check

# Watch output
make serial
```

### Development Cycle

```bash
# Edit code
vim lib/calendar_display.py

# Upload changes
make upload

# Device auto-reloads in a few seconds
# Watch output for results
make serial

# If you need REPL access
make repl
```

### Troubleshooting

```bash
# Check what's on device
make status

# Verify all files uploaded correctly
make check

# Test API connectivity
make test-api API_KEY=your-key USER_ID=1

# Watch serial output for errors
make serial
```

### Debugging Issue

```bash
# Enable debug mode in code.py
make debug              # Shows current setting

# Edit code.py and set DEBUG = True

# Upload and watch
make upload && make serial

# For interactive testing
make repl
```

## Troubleshooting Make Commands

### "Error: No serial device found"

**Solution:** Connect device via USB and check it appears:

```bash
# List serial devices
ls /dev/tty.* 2>/dev/null     # macOS/Linux
wmic logicaldisk list brief    # Windows
```

### "Error: CIRCUITPY not mounted"

**Solution:** 
1. Connect device via USB
2. Double-click the reset button
3. CIRCUITPY drive should appear
4. Retry command

### "make: command not found"

**Solution:** Install `make`:

```bash
# macOS (with Homebrew)
brew install make

# Ubuntu/Debian
sudo apt-get install make

# CentOS/RHEL
sudo yum install make

# Windows (with chocolatey)
choco install make
```

### "Permission denied" on CIRCUITPY

**Solution:** Device might be read-only. Try:

```bash
# Unmount and remount
umount /Volumes/CIRCUITPY     # macOS
diskutil mount CIRCUITPY       # macOS

# Or restart device:
# Press reset button
```

## Platform-Specific Notes

### macOS

- Serial port typically: `/dev/tty.usbmodem1`
- CIRCUITPY mount: `/Volumes/CIRCUITPY`
- Use `screen` for serial (comes pre-installed)

### Linux

- Serial port typically: `/dev/ttyACM0`
- CIRCUITPY mount: `/media/$USER/CIRCUITPY`
- Install `screen`: `sudo apt-get install screen`

### Windows

- Serial port: `COM3`, `COM4`, etc.
- CIRCUITPY mount: Drive letter (e.g., `D:\`)
- Use `putty` or `minicom` for serial
- Makefile syntax may need adjustment

## Advanced Usage

### Custom Device Port

```bash
# If auto-detection doesn't work
DEVICE_PORT=/dev/ttyACM0 make serial
```

### Custom Baud Rate

```bash
# Change in Makefile (default: 115200)
BAUD_RATE=9600 make serial
```

### Custom Mount Point

```bash
MOUNT=/custom/mount/path make upload
```

## Tips & Tricks

### Quick Upload + Monitor

```bash
make upload && make serial
```

### Check Status + Upload + Monitor

```bash
make status && make upload && make serial
```

### Test API and Show Pretty JSON

```bash
make test-api API_KEY=key USER_ID=1 | head -20
```

### Verify Setup is Complete

```bash
make status && make check
```

## Reference

### Command Categories

**Quick Start:**
- `make config` - Setup
- `make upload` - Deploy
- `make serial` - Monitor

**Debugging:**
- `make debug` - Enable verbose logging
- `make repl` - Interactive shell
- `make serial` - View output

**Verification:**
- `make status` - Device status
- `make check` - File verification
- `make test` - API connectivity

**Maintenance:**
- `make clean` - Cleanup
- `make help` - Show all commands

### Default Values

```makefile
BAUD_RATE = 115200
CIRCUITPY_MOUNT = /Volumes/CIRCUITPY (macOS)
DEVICE_PORT = auto-detected
```

---

**For more help:** Run `make help` or see QUICK_REFERENCE.md
