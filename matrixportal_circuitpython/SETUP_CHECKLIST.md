# CircuitPython Setup Checklist

Use this checklist to track your CircuitPython installation progress.

## Phase 1: Hardware & CircuitPython Installation

- [ ] **Have MatrixPortal M4 board available**
- [ ] **Have USB-C cable**
- [ ] **Download CircuitPython**
  - [ ] Visit https://circuitpython.org/board/matrixportal_m4/
  - [ ] Download latest stable (8.x+)
  - [ ] Save to downloads folder
- [ ] **Install CircuitPython to device**
  - [ ] Double-click reset button on MatrixPortal
  - [ ] Drag .uf2 file to MATRIXPORTAL drive
  - [ ] Wait for reboot
  - [ ] Verify CIRCUITPY drive appears
- [ ] **Verify CircuitPython installed**
  - [ ] Connect serial monitor (115200 baud)
  - [ ] See CircuitPython boot message
  - [ ] Can see REPL prompt (>>>)

## Phase 2: Library Installation

- [ ] **Download Adafruit Library Bundle**
  - [ ] Visit https://circuitpython.org/libraries
  - [ ] Download matching CircuitPython version
  - [ ] Extract ZIP file
- [ ] **Copy required libraries**
  - [ ] Create CIRCUITPY/lib folder (if doesn't exist)
  - [ ] Copy adafruit_requests.mpy
  - [ ] Copy adafruit_display_text/ folder
  - [ ] Copy adafruit_bitmap_font/ folder
  - [ ] Verify folders appear in CIRCUITPY/lib/
- [ ] **Verify libraries loaded**
  - [ ] In REPL, type: `import adafruit_requests`
  - [ ] Should show no error
  - [ ] In REPL, type: `import adafruit_display_text`
  - [ ] Should show no error

## Phase 3: Application Files

- [ ] **Copy application files to CIRCUITPY/**
  - [ ] Copy code.py (main application)
  - [ ] Copy config.example.py
  - [ ] Create lib/ folder
  - [ ] Copy lib/wifi_manager.py
  - [ ] Copy lib/api_client.py
  - [ ] Copy lib/calendar_display.py
- [ ] **Verify file structure**
  ```
  CIRCUITPY/
  ├── code.py
  ├── config.example.py
  ├── lib/
  │   ├── wifi_manager.py
  │   ├── api_client.py
  │   └── calendar_display.py
  └── (Adafruit libs...)
  ```

## Phase 4: Configuration

- [ ] **Create secrets.py**
  - [ ] Copy config.example.py to secrets.py
  - [ ] Edit secrets.py
- [ ] **Configure WiFi**
  - [ ] [ ] Set WIFI_SSID = "your-network"
  - [ ] [ ] Set WIFI_PASSWORD = "password"
- [ ] **Configure API**
  - [ ] Get API key from backend admin
  - [ ] Set API_BASE_URL (production or local)
  - [ ] Set API_KEY = "your-key"
  - [ ] Set USER_ID = 1 (or your user ID)
- [ ] **Configure Display**
  - [ ] Set REFRESH_INTERVAL_SECONDS = 300 (optional)
  - [ ] Set DISPLAY_BRIGHTNESS = 1.0 (optional)
- [ ] **Verify secrets.py**
  - [ ] Open secrets.py in editor
  - [ ] Confirm all values are filled in
  - [ ] Verify WiFi credentials are correct
  - [ ] Verify API URL is correct (https://)

## Phase 5: Testing

- [ ] **Boot device and check serial output**
  - [ ] Connect serial monitor (115200 baud)
  - [ ] Device should show startup messages
  - [ ] See "[INFO] === Matrix Miles (CircuitPython) ==="
- [ ] **Monitor WiFi connection**
  - [ ] See "[INFO] Connecting to WiFi..."
  - [ ] Wait 5-10 seconds
  - [ ] See "[INFO] WiFi connected: 192.168.x.x"
- [ ] **Monitor API requests**
  - [ ] See "[INFO] Fetching activities..."
  - [ ] See "[INFO] API response received: N activities"
  - [ ] No error messages
- [ ] **Check display**
  - [ ] Matrix displays activity data
  - [ ] Display updates every 5 minutes (or your interval)
  - [ ] No flickering or errors
- [ ] **Monitor for 5+ minutes**
  - [ ] No crashes or restarts
  - [ ] Serial output continues normally
  - [ ] Display updates consistently

## Phase 6: Troubleshooting (if needed)

### WiFi Connection Fails
- [ ] Verify SSID and password in secrets.py
- [ ] Check network is 2.4GHz (not 5GHz)
- [ ] Restart device (press reset button)
- [ ] Move device closer to router
- [ ] Check WiFi signal strength (RSSI in serial output)

### API Request Fails
- [ ] Test API with curl:
  ```bash
  curl -H "X-API-Key: your-key" https://matrix-miles-production.up.railway.app/api/activities/recent/1
  ```
- [ ] Verify API_BASE_URL is correct in secrets.py
- [ ] Verify API_KEY is correct
- [ ] Verify USER_ID exists in backend
- [ ] Check backend is running: https://matrix-miles-production.up.railway.app/health

### No Display Output
- [ ] Verify adafruit libraries are installed
- [ ] Check CIRCUITPY/lib/ folder contents
- [ ] Restart device
- [ ] Check serial for errors

### Device Won't Boot
- [ ] Check code.py for syntax errors
- [ ] Open REPL (Ctrl+C while device is running)
- [ ] Try importing modules:
  ```python
  >>> import code
  ```
- [ ] If error, fix code.py and save
- [ ] Device should auto-reload
- [ ] Press reset button if stuck

## Phase 7: Verification

- [ ] **Test for 24 hours**
  - [ ] Leave device running
  - [ ] Spot-check serial output periodically
  - [ ] Verify no error patterns
- [ ] **Manual API test**
  - [ ] Add activity to Strava
  - [ ] Wait for sync (usually automatic)
  - [ ] Check if new activity appears on display
  - [ ] Wait through one refresh cycle
- [ ] **Test WiFi reconnection**
  - [ ] Turn off WiFi on your phone/router
  - [ ] Watch serial output
  - [ ] Device should attempt to reconnect
  - [ ] Turn WiFi back on
  - [ ] Device should reconnect within 30 seconds

## Phase 8: Customization (Optional)

- [ ] **Adjust refresh interval**
  - [ ] Edit secrets.py
  - [ ] Change REFRESH_INTERVAL_SECONDS
  - [ ] Save and device reloads automatically
- [ ] **Adjust display brightness**
  - [ ] Edit secrets.py
  - [ ] Change DISPLAY_BRIGHTNESS (0.0 to 1.0)
  - [ ] Save and device reloads
- [ ] **Customize display rendering**
  - [ ] Edit lib/calendar_display.py
  - [ ] Modify colors, layout, text
  - [ ] Device auto-reloads changes
- [ ] **Add debug logging**
  - [ ] Edit code.py
  - [ ] Set DEBUG = True for more output
  - [ ] Save and observe serial

## Final Checklist

- [ ] **Device running 24+ hours without errors**
- [ ] **Activities display correctly**
- [ ] **WiFi reconnects automatically**
- [ ] **Display updates on schedule**
- [ ] **No manual intervention needed**
- [ ] **Ready for production deployment**

## What to Do Next

After successful setup:

1. **Monitor for a week** to ensure stability
2. **Customize display** if desired (colors, layout)
3. **Add activity filtering** (only show runs, etc.)
4. **Implement statistics** (weekly totals, streaks)
5. **Add button controls** (navigate between views)
6. **Set up power management** (deep sleep, scheduled wakeup)

## Support & Help

- **Can't connect to WiFi?** → See GETTING_STARTED.md > Troubleshooting
- **API errors?** → See docs/API_ENDPOINTS.md
- **Questions about CircuitPython?** → See CIRCUITPYTHON_VS_CPP.md
- **Device won't boot?** → See QUICK_REFERENCE.md > Emergency Reset
- **Want to debug?** → See QUICK_REFERENCE.md > REPL Testing Commands

---

**Bookmark this checklist!** You can refer back to it if you need to troubleshoot later. 📋
