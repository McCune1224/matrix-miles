# Quick Reference Card

**Copy this after setup to your MatrixPortal for quick reference.**

## Installation Checklist

- [ ] Download CircuitPython 8.x for MatrixPortal M4
- [ ] Double-click reset button, drag .uf2 file to MATRIXPORTAL drive
- [ ] Download library bundle, copy required libraries to CIRCUITPY/lib/
- [ ] Copy code.py, lib/*.py, secrets.py to CIRCUITPY/
- [ ] Edit secrets.py with your WiFi and API credentials
- [ ] Open serial monitor (115200 baud)
- [ ] Device should show "WiFi: Connecting..." on display
- [ ] Check serial for "WiFi connected: 192.168.x.x"
- [ ] Wait for "API response received: N activities"
- [ ] Verify activities display on matrix

## Required Libraries (from Adafruit Bundle)

```
lib/
├── adafruit_requests.mpy
├── adafruit_display_text/
├── adafruit_bitmap_font/
├── adafruit_bus_device/
└── neopixel.mpy (optional)
```

## Configuration (secrets.py)

```python
WIFI_SSID = "your-network"
WIFI_PASSWORD = "password"
API_BASE_URL = "https://matrix-miles-production.up.railway.app"
API_KEY = "your-key"
USER_ID = 1
REFRESH_INTERVAL_SECONDS = 300  # 5 minutes
```

## Serial Debug Output

```
[0.1] [INFO] === Matrix Miles (CircuitPython) ===
[1.2] [INFO] Connecting to WiFi...
[3.5] [INFO] WiFi connected: 192.168.1.100
[5.0] [INFO] Fetching activities...
[6.2] [INFO] API response received: 3 activities
[6.3] [INFO] Rendering 3 activities
[6.4] [INFO] Calendar rendered successfully
```

## Troubleshooting Quick Links

| Issue | Solution |
|-------|----------|
| Device won't boot | Check code.py syntax (Ctrl+C in REPL) |
| WiFi fails | Verify SSID/password, check 2.4GHz |
| API error | Check base URL, test with curl |
| No display | Check lib/ has required libraries |
| SSL error | Verify HTTPS in API_BASE_URL |

## REPL Testing Commands

```python
# Check imports
>>> import wifi
>>> import board
>>> from lib.api_client import APIClient

# Check WiFi
>>> wifi.radio.connected
True
>>> str(wifi.radio.ipv4_address)
'192.168.1.100'

# Test API
>>> from lib.api_client import APIClient
>>> client = APIClient("https://...", "key", 1)
>>> activities = client.get_recent_activities()
>>> len(activities)
3
```

## File Locations on Device

```
CIRCUITPY/
├── code.py                 # Main application (code.py auto-runs)
├── secrets.py              # Your configuration (copy from config.example.py)
├── lib/
│   ├── wifi_manager.py     # WiFi handling
│   ├── api_client.py       # API requests
│   ├── calendar_display.py # Display rendering
│   └── (Adafruit libraries...)
└── boot_out.txt            # CircuitPython boot log
```

## Useful CircuitPython Commands

```python
# View boot messages
>>> import storage
>>> storage.remount("/", readonly=False)

# Reload code without restart
>>> import supervisor
>>> supervisor.reload()

# Check free memory
>>> import gc
>>> gc.mem_free()
140288

# Get board info
>>> import board
>>> print(board.DISPLAY)
```

## Common Errors & Fixes

**"ImportError: No module named 'lib'"**
- Solution: Make sure lib/ folder exists in CIRCUITPY/

**"WiFi connection failed"**
- Solution: Check SSID/password, try 2.4GHz network

**"X-API-Key: Invalid"**
- Solution: Verify API_KEY in secrets.py matches backend config

**"SSL: CERTIFICATE_VERIFY_FAILED"**
- Solution: Use HTTPS URL, check certificate is valid

## Files to Never Delete

- `code.py` - Application won't run
- `boot.py` - Needed for startup (auto-generated)
- `CIRCUITPY/lib/` folder - Libraries needed

## Files Safe to Delete (Will Be Recreated)

- `.Trashes/` folder
- `System Volume Information/` (Mac)
- `.fseventsd` (Mac)

## Performance Tips

1. Increase `REFRESH_INTERVAL_SECONDS` to reduce network traffic
2. Set `DISPLAY_BRIGHTNESS = 0.5` to save power
3. Use `DEBUG = False` in code.py to reduce serial output
4. Monitor serial for error patterns (helps debugging)

## Next Customizations

After basic setup works:
1. Customize calendar colors (edit calendar_display.py)
2. Add activity type filtering
3. Display weekly/monthly statistics
4. Implement button controls
5. Add power management/deep sleep

## Support Resources

- **CircuitPython Docs:** https://circuitpython.readthedocs.io/
- **MatrixPortal Guide:** https://learn.adafruit.com/adafruit-matrixportal-m4/
- **Adafruit Support:** https://learn.adafruit.com/
- **Your Backend API:** See docs/API_ENDPOINTS.md

## Emergency Reset

If device won't boot:
1. Unplug USB
2. Press and hold reset button
3. Plug USB back in (keeping button held)
4. Release after LED changes color
5. Device will boot in safe mode
6. Fix code.py and try again

---

**Bookmark this page for quick reference during setup!** 📌
