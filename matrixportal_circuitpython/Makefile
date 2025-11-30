# CircuitPython MatrixPortal M4 Development Makefile
# Provides convenient commands for development, debugging, and deployment

.PHONY: help serial monitor upload sync copy repl debug test clean status

# Configuration
BAUD_RATE := 115200
CIRCUITPY_MOUNT := /run/media/$(USER)/CIRCUITPY
CIRCUITPY_MOUNT_ALT := /media/$(USER)/CIRCUITPY
DEVICE_PORT := $(shell ls -t /dev/tty.* 2>/dev/null | head -1)
DEVICE_PORT_ALT := $(shell ls -t /dev/ttyACM* 2>/dev/null | head -1)

# Determine which mount point exists
ifeq ($(shell test -d $(CIRCUITPY_MOUNT) && echo 1),1)
  MOUNT := $(CIRCUITPY_MOUNT)
else ifeq ($(shell test -d $(CIRCUITPY_MOUNT_ALT) && echo 1),1)
  MOUNT := $(CIRCUITPY_MOUNT_ALT)
else
  MOUNT := 
endif

help:
	@echo "═══════════════════════════════════════════════════════════════════════════"
	@echo "CircuitPython MatrixPortal M4 - Development Commands"
	@echo "═══════════════════════════════════════════════════════════════════════════"
	@echo ""
	@echo "MONITORING & DEBUGGING:"
	@echo "  make serial          View serial output (115200 baud)"
	@echo "  make monitor         Alias for serial"
	@echo "  make repl            Interactive REPL via serial"
	@echo "  make debug           Debug mode (verbose logging)"
	@echo ""
	@echo "FILE MANAGEMENT:"
	@echo "  make upload          Upload code.py and lib/ to device"
	@echo "  make sync            Sync all Python files to device"
	@echo "  make copy            Same as upload"
	@echo "  make config          Setup secrets.py from template"
	@echo ""
	@echo "VERIFICATION:"
	@echo "  make status          Check device and mount status"
	@echo "  make test            Test API connectivity"
	@echo "  make check           Verify file structure on device"
	@echo ""
	@echo "UTILITIES:"
	@echo "  make clean           Remove local temp files"
	@echo "  make help            Show this help message"
	@echo ""
	@echo "EXAMPLES:"
	@echo "  # View real-time output:"
	@echo "  make serial"
	@echo ""
	@echo "  # Upload code and watch output:"
	@echo "  make upload && make serial"
	@echo ""
	@echo "  # Setup configuration first time:"
	@echo "  make config && make upload"
	@echo ""
	@echo "  # Test API without device:"
	@echo "  make test"
	@echo ""
	@echo "═══════════════════════════════════════════════════════════════════════════"

serial:
	@echo "Connecting to serial monitor ($(BAUD_RATE) baud)..."
	@echo "Press Ctrl+A then Ctrl+X to exit (screen)"
	@echo "Or Ctrl+] to exit (telnet)"
	@echo ""
	@if [ -z "$(DEVICE_PORT)" ] && [ -z "$(DEVICE_PORT_ALT)" ]; then \
		echo "Error: No serial device found"; \
		echo "Check that your MatrixPortal M4 is connected via USB"; \
		exit 1; \
	fi
	@if command -v screen >/dev/null 2>&1; then \
		screen $(DEVICE_PORT)$(DEVICE_PORT_ALT) $(BAUD_RATE); \
	elif command -v picocom >/dev/null 2>&1; then \
		picocom -b $(BAUD_RATE) $(DEVICE_PORT)$(DEVICE_PORT_ALT); \
	elif command -v minicom >/dev/null 2>&1; then \
		minicom -b $(BAUD_RATE) -D $(DEVICE_PORT)$(DEVICE_PORT_ALT); \
	else \
		echo "Error: No serial monitor tool found"; \
		echo "Install one of: screen, picocom, or minicom"; \
		exit 1; \
	fi

monitor: serial

# Interactive REPL
repl:
	@echo "Entering CircuitPython REPL..."
	@echo "Type Ctrl+D to soft reset"
	@echo "Type Ctrl+C to stop running code"
	@echo "Type exit() to quit REPL"
	@echo ""
	@if [ -z "$(DEVICE_PORT)" ] && [ -z "$(DEVICE_PORT_ALT)" ]; then \
		echo "Error: No serial device found"; \
		exit 1; \
	fi
	@python3 -m serial.tools.miniterm $(DEVICE_PORT)$(DEVICE_PORT_ALT) $(BAUD_RATE) --raw

debug:
	@echo "Starting debug mode..."
	@echo "Edit code.py and set DEBUG = True near the top"
	@echo "Then run: make upload && make serial"
	@echo ""
	@if [ ! -f "code.py" ]; then \
		echo "Error: code.py not found in current directory"; \
		exit 1; \
	fi
	@echo "Current DEBUG setting:"
	@grep -n "DEBUG = " code.py

# Upload code to device
upload: check-mount
	@echo "Uploading files to device..."
	@echo ""
	@if [ ! -f "code.py" ]; then \
		echo "Error: code.py not found"; \
		exit 1; \
	fi
	@if [ ! -d "lib" ]; then \
		echo "Error: lib/ directory not found"; \
		exit 1; \
	fi
	@echo "Copying code.py..."
	@cp code.py "$(MOUNT)/"
	@echo "✓ code.py copied"
	@echo ""
	@echo "Copying lib/ directory..."
	@cp -r lib/* "$(MOUNT)/lib/"
	@echo "✓ lib/ copied"
	@echo ""
	@echo "✓ Upload complete!"
	@echo "Device will auto-reload in a few seconds..."

sync: upload

copy: upload

# Setup configuration
config:
	@echo "Setting up configuration..."
	@echo ""
	@if [ ! -f "config.example.py" ]; then \
		echo "Error: config.example.py not found"; \
		exit 1; \
	fi
	@if [ -f "secrets.py" ]; then \
		echo "secrets.py already exists"; \
		echo "Backup existing file? (y/n)"; \
		read -r response; \
		if [ "$$response" = "y" ]; then \
			cp secrets.py secrets.py.backup; \
			echo "Backed up to secrets.py.backup"; \
		fi; \
	fi
	@cp config.example.py secrets.py
	@echo "✓ secrets.py created from template"
	@echo ""
	@echo "Next steps:"
	@echo "  1. Edit secrets.py with your configuration:"
	@echo "     - WiFi SSID and password"
	@echo "     - API base URL"
	@echo "     - API key"
	@echo "     - User ID"
	@echo ""
	@echo "  2. Run: make upload"
	@echo "  3. Run: make serial (to verify)"

# Check device status
status:
	@echo "Device Status Check"
	@echo "═══════════════════════════════════════════════════════════"
	@echo ""
	@echo "Mount Point:"
	@if [ -n "$(MOUNT)" ]; then \
		echo "  ✓ CIRCUITPY mounted at: $(MOUNT)"; \
		echo ""; \
		echo "  Files on device:"; \
		ls -1 "$(MOUNT)/" | sed 's/^/    /'; \
	else \
		echo "  ✗ CIRCUITPY not mounted"; \
		echo "    Expected at: $(CIRCUITPY_MOUNT) or $(CIRCUITPY_MOUNT_ALT)"; \
		echo "    Connect device and check mount status"; \
	fi
	@echo ""
	@echo "Serial Port:"
	@if [ -n "$(DEVICE_PORT)" ]; then \
		echo "  ✓ Device found at: $(DEVICE_PORT)"; \
	elif [ -n "$(DEVICE_PORT_ALT)" ]; then \
		echo "  ✓ Device found at: $(DEVICE_PORT_ALT)"; \
	else \
		echo "  ✗ No serial device found"; \
		echo "    Connect device via USB"; \
	fi
	@echo ""
	@echo "Local Files:"
	@echo "  code.py: $(shell test -f code.py && echo '✓' || echo '✗')"
	@echo "  lib/: $(shell test -d lib && echo '✓' || echo '✗')"
	@echo "  config.example.py: $(shell test -f config.example.py && echo '✓' || echo '✗')"
	@echo "  secrets.py: $(shell test -f secrets.py && echo '✓' || echo 'not configured')"
	@echo ""

# Verify file structure on device
check:
	@echo "Verifying device file structure..."
	@echo ""
	@if [ -z "$(MOUNT)" ]; then \
		echo "Error: CIRCUITPY not mounted"; \
		exit 1; \
	fi
	@echo "Expected files:"
	@if [ -f "$(MOUNT)/code.py" ]; then \
		echo "  ✓ code.py ($(shell wc -l < $(MOUNT)/code.py) lines)"; \
	else \
		echo "  ✗ code.py missing"; \
	fi
	@if [ -f "$(MOUNT)/lib/api_client.py" ]; then \
		echo "  ✓ lib/api_client.py ($(shell wc -l < $(MOUNT)/lib/api_client.py) lines)"; \
	else \
		echo "  ✗ lib/api_client.py missing"; \
	fi
	@if [ -f "$(MOUNT)/lib/wifi_manager.py" ]; then \
		echo "  ✓ lib/wifi_manager.py ($(shell wc -l < $(MOUNT)/lib/wifi_manager.py) lines)"; \
	else \
		echo "  ✗ lib/wifi_manager.py missing"; \
	fi
	@if [ -f "$(MOUNT)/lib/calendar_display.py" ]; then \
		echo "  ✓ lib/calendar_display.py ($(shell wc -l < $(MOUNT)/lib/calendar_display.py) lines)"; \
	else \
		echo "  ✗ lib/calendar_display.py missing"; \
	fi
	@echo ""
	@echo "Required Adafruit libraries:"
	@if [ -f "$(MOUNT)/lib/adafruit_requests.mpy" ]; then \
		echo "  ✓ adafruit_requests.mpy"; \
	else \
		echo "  ✗ adafruit_requests.mpy missing"; \
	fi
	@if [ -d "$(MOUNT)/lib/adafruit_display_text" ]; then \
		echo "  ✓ adafruit_display_text/"; \
	else \
		echo "  ✗ adafruit_display_text/ missing"; \
	fi
	@if [ -d "$(MOUNT)/lib/adafruit_bitmap_font" ]; then \
		echo "  ✓ adafruit_bitmap_font/"; \
	else \
		echo "  ✗ adafruit_bitmap_font/ missing"; \
	fi
	@echo ""

# Test API connectivity
test:
	@echo "Testing API Connectivity"
	@echo "═══════════════════════════════════════════════════════════"
	@echo ""
	@if [ ! -f "secrets.py" ]; then \
		echo "Note: secrets.py not found. Using example values."; \
		echo "Run 'make config' first to set up actual credentials."; \
		echo ""; \
		API_BASE_URL="https://matrix-miles-production.up.railway.app"; \
		API_KEY="test-key"; \
	else \
		API_BASE_URL=$$(grep "API_BASE_URL" secrets.py | cut -d'"' -f2); \
		API_KEY=$$(grep "API_KEY" secrets.py | cut -d'"' -f2); \
	fi
	@echo "Testing API base URL: $$API_BASE_URL"
	@echo ""
	@echo "Health check:"
	@curl -s -I "$$API_BASE_URL/health" | head -3 || echo "Failed to connect"
	@echo ""
	@echo "API test (this requires valid API key):"
	@echo "  Run: make test-api API_KEY=your-key USER_ID=1"
	@echo ""

test-api:
	@if [ -z "$(API_KEY)" ] || [ -z "$(USER_ID)" ]; then \
		echo "Usage: make test-api API_KEY=<key> USER_ID=<id>"; \
		exit 1; \
	fi
	@echo "Testing API request..."
	@API_BASE_URL=$$(grep "API_BASE_URL" secrets.py 2>/dev/null | cut -d'"' -f2 || echo "https://matrix-miles-production.up.railway.app"); \
	echo "Base URL: $$API_BASE_URL"; \
	echo "User ID: $(USER_ID)"; \
	echo ""; \
	echo "Fetching activities..."; \
	curl -s -H "X-API-Key: $(API_KEY)" \
	     "$$API_BASE_URL/api/activities/recent/$(USER_ID)" | python3 -m json.tool

# Clean local temporary files
clean:
	@echo "Cleaning up..."
	@find . -type f -name "*.pyc" -delete
	@find . -type d -name "__pycache__" -delete
	@find . -type f -name "*.swp" -delete
	@find . -type f -name "*~" -delete
	@echo "✓ Cleaned"

# Internal target to check mount
check-mount:
	@if [ -z "$(MOUNT)" ]; then \
		echo "Error: CIRCUITPY not mounted"; \
		echo ""; \
		echo "Please:"; \
		echo "  1. Connect your MatrixPortal M4 via USB"; \
		echo "  2. Double-click the reset button"; \
		echo "  3. Wait for CIRCUITPY drive to appear"; \
		exit 1; \
	fi

# Platform detection
.PHONY: platform
platform:
	@echo "Platform Detection"
	@echo "OS: $$(uname -s)"
	@echo "Mount: $(MOUNT)"
	@echo "Device: $(DEVICE_PORT)$(DEVICE_PORT_ALT)"
