# CircuitPython MatrixPortal M4 Development Makefile
# Provides convenient commands for development, debugging, and deployment

.PHONY: help serial monitor upload sync copy repl debug test clean status

# Configuration
BAUD_RATE := 115200
CIRCUITPY_MOUNT := /Volumes/CIRCUITPY
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

# Color output
RED := \033[0;31m
GREEN := \033[0;32m
YELLOW := \033[0;33m
BLUE := \033[0;34m
NC := \033[0m # No Color

help:
	@echo "$(BLUE)═══════════════════════════════════════════════════════════════════════════$(NC)"
	@echo "$(BLUE)CircuitPython MatrixPortal M4 - Development Commands$(NC)"
	@echo "$(BLUE)═══════════════════════════════════════════════════════════════════════════$(NC)"
	@echo ""
	@echo "$(GREEN)MONITORING & DEBUGGING:$(NC)"
	@echo "  make serial          View serial output (115200 baud)"
	@echo "  make monitor         Alias for serial"
	@echo "  make repl            Interactive REPL via serial"
	@echo "  make debug           Debug mode (verbose logging)"
	@echo ""
	@echo "$(GREEN)FILE MANAGEMENT:$(NC)"
	@echo "  make upload          Upload code.py and lib/ to device"
	@echo "  make sync            Sync all Python files to device"
	@echo "  make copy            Same as upload"
	@echo "  make config          Setup secrets.py from template"
	@echo ""
	@echo "$(GREEN)VERIFICATION:$(NC)"
	@echo "  make status          Check device and mount status"
	@echo "  make test            Test API connectivity"
	@echo "  make check           Verify file structure on device"
	@echo ""
	@echo "$(GREEN)UTILITIES:$(NC)"
	@echo "  make clean           Remove local temp files"
	@echo "  make help            Show this help message"
	@echo ""
	@echo "$(YELLOW)EXAMPLES:$(NC)"
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
	@echo "$(BLUE)═══════════════════════════════════════════════════════════════════════════$(NC)"

serial:
	@echo "$(GREEN)Connecting to serial monitor ($(BAUD_RATE) baud)...$(NC)"
	@echo "$(YELLOW)Press Ctrl+A then Ctrl+X to exit (screen)$(NC)"
	@echo "$(YELLOW)Or Ctrl+] to exit (telnet)$(NC)"
	@echo ""
	@if [ -z "$(DEVICE_PORT)" ] && [ -z "$(DEVICE_PORT_ALT)" ]; then \
		echo "$(RED)Error: No serial device found$(NC)"; \
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
		echo "$(RED)Error: No serial monitor tool found$(NC)"; \
		echo "Install one of: screen, picocom, or minicom"; \
		exit 1; \
	fi

monitor: serial

# Interactive REPL
repl:
	@echo "$(GREEN)Entering CircuitPython REPL...$(NC)"
	@echo "$(YELLOW)Type Ctrl+D to soft reset$(NC)"
	@echo "$(YELLOW)Type Ctrl+C to stop running code$(NC)"
	@echo "$(YELLOW)Type exit() to quit REPL$(NC)"
	@echo ""
	@if [ -z "$(DEVICE_PORT)" ] && [ -z "$(DEVICE_PORT_ALT)" ]; then \
		echo "$(RED)Error: No serial device found$(NC)"; \
		exit 1; \
	fi
	@python3 -m serial.tools.miniterm $(DEVICE_PORT)$(DEVICE_PORT_ALT) $(BAUD_RATE) --raw

debug:
	@echo "$(GREEN)Starting debug mode...$(NC)"
	@echo "$(YELLOW)Edit code.py and set DEBUG = True near the top$(NC)"
	@echo "$(YELLOW)Then run: make upload && make serial$(NC)"
	@echo ""
	@if [ ! -f "code.py" ]; then \
		echo "$(RED)Error: code.py not found in current directory$(NC)"; \
		exit 1; \
	fi
	@echo "Current DEBUG setting:"
	@grep -n "DEBUG = " code.py

# Upload code to device
upload: check-mount
	@echo "$(GREEN)Uploading files to device...$(NC)"
	@echo ""
	@if [ ! -f "code.py" ]; then \
		echo "$(RED)Error: code.py not found$(NC)"; \
		exit 1; \
	fi
	@if [ ! -d "lib" ]; then \
		echo "$(RED)Error: lib/ directory not found$(NC)"; \
		exit 1; \
	fi
	@echo "$(BLUE)Copying code.py...$(NC)"
	@cp code.py "$(MOUNT)/"
	@echo "$(GREEN)✓ code.py copied$(NC)"
	@echo ""
	@echo "$(BLUE)Copying lib/ directory...$(NC)"
	@cp -r lib/* "$(MOUNT)/lib/"
	@echo "$(GREEN)✓ lib/ copied$(NC)"
	@echo ""
	@echo "$(GREEN)✓ Upload complete!$(NC)"
	@echo "$(YELLOW)Device will auto-reload in a few seconds...$(NC)"

sync: upload

copy: upload

# Setup configuration
config:
	@echo "$(GREEN)Setting up configuration...$(NC)"
	@echo ""
	@if [ ! -f "config.example.py" ]; then \
		echo "$(RED)Error: config.example.py not found$(NC)"; \
		exit 1; \
	fi
	@if [ -f "secrets.py" ]; then \
		echo "$(YELLOW)secrets.py already exists$(NC)"; \
		echo "Backup existing file? (y/n)"; \
		read -r response; \
		if [ "$$response" = "y" ]; then \
			cp secrets.py secrets.py.backup; \
			echo "$(GREEN)Backed up to secrets.py.backup$(NC)"; \
		fi; \
	fi
	@cp config.example.py secrets.py
	@echo "$(GREEN)✓ secrets.py created from template$(NC)"
	@echo ""
	@echo "$(YELLOW)Next steps:$(NC)"
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
	@echo "$(BLUE)Device Status Check$(NC)"
	@echo "$(BLUE)═══════════════════════════════════════════════════════════$(NC)"
	@echo ""
	@echo "$(YELLOW)Mount Point:$(NC)"
	@if [ -n "$(MOUNT)" ]; then \
		echo "  $(GREEN)✓ CIRCUITPY mounted at: $(MOUNT)$(NC)"; \
		echo ""; \
		echo "  Files on device:"; \
		ls -1 "$(MOUNT)/" | sed 's/^/    /'; \
	else \
		echo "  $(RED)✗ CIRCUITPY not mounted$(NC)"; \
		echo "    Expected at: $(CIRCUITPY_MOUNT) or $(CIRCUITPY_MOUNT_ALT)"; \
		echo "    Connect device and check mount status"; \
	fi
	@echo ""
	@echo "$(YELLOW)Serial Port:$(NC)"
	@if [ -n "$(DEVICE_PORT)" ]; then \
		echo "  $(GREEN)✓ Device found at: $(DEVICE_PORT)$(NC)"; \
	elif [ -n "$(DEVICE_PORT_ALT)" ]; then \
		echo "  $(GREEN)✓ Device found at: $(DEVICE_PORT_ALT)$(NC)"; \
	else \
		echo "  $(RED)✗ No serial device found$(NC)"; \
		echo "    Connect device via USB"; \
	fi
	@echo ""
	@echo "$(YELLOW)Local Files:$(NC)"
	@echo "  code.py: $(shell test -f code.py && echo '$(GREEN)✓$(NC)' || echo '$(RED)✗$(NC)')"
	@echo "  lib/: $(shell test -d lib && echo '$(GREEN)✓$(NC)' || echo '$(RED)✗$(NC)')"
	@echo "  config.example.py: $(shell test -f config.example.py && echo '$(GREEN)✓$(NC)' || echo '$(RED)✗$(NC)')"
	@echo "  secrets.py: $(shell test -f secrets.py && echo '$(GREEN)✓$(NC)' || echo '$(YELLOW)not configured$(NC)')"
	@echo ""

# Verify file structure on device
check:
	@echo "$(BLUE)Verifying device file structure...$(NC)"
	@echo ""
	@if [ -z "$(MOUNT)" ]; then \
		echo "$(RED)Error: CIRCUITPY not mounted$(NC)"; \
		exit 1; \
	fi
	@echo "$(YELLOW)Expected files:$(NC)"
	@if [ -f "$(MOUNT)/code.py" ]; then \
		echo "  $(GREEN)✓ code.py$(NC) ($(shell wc -l < $(MOUNT)/code.py) lines)"; \
	else \
		echo "  $(RED)✗ code.py missing$(NC)"; \
	fi
	@if [ -f "$(MOUNT)/lib/api_client.py" ]; then \
		echo "  $(GREEN)✓ lib/api_client.py$(NC) ($(shell wc -l < $(MOUNT)/lib/api_client.py) lines)"; \
	else \
		echo "  $(RED)✗ lib/api_client.py missing$(NC)"; \
	fi
	@if [ -f "$(MOUNT)/lib/wifi_manager.py" ]; then \
		echo "  $(GREEN)✓ lib/wifi_manager.py$(NC) ($(shell wc -l < $(MOUNT)/lib/wifi_manager.py) lines)"; \
	else \
		echo "  $(RED)✗ lib/wifi_manager.py missing$(NC)"; \
	fi
	@if [ -f "$(MOUNT)/lib/calendar_display.py" ]; then \
		echo "  $(GREEN)✓ lib/calendar_display.py$(NC) ($(shell wc -l < $(MOUNT)/lib/calendar_display.py) lines)"; \
	else \
		echo "  $(RED)✗ lib/calendar_display.py missing$(NC)"; \
	fi
	@echo ""
	@echo "$(YELLOW)Required Adafruit libraries:$(NC)"
	@if [ -f "$(MOUNT)/lib/adafruit_requests.mpy" ]; then \
		echo "  $(GREEN)✓ adafruit_requests.mpy$(NC)"; \
	else \
		echo "  $(RED)✗ adafruit_requests.mpy missing$(NC)"; \
	fi
	@if [ -d "$(MOUNT)/lib/adafruit_display_text" ]; then \
		echo "  $(GREEN)✓ adafruit_display_text/$(NC)"; \
	else \
		echo "  $(RED)✗ adafruit_display_text/ missing$(NC)"; \
	fi
	@if [ -d "$(MOUNT)/lib/adafruit_bitmap_font" ]; then \
		echo "  $(GREEN)✓ adafruit_bitmap_font/$(NC)"; \
	else \
		echo "  $(RED)✗ adafruit_bitmap_font/ missing$(NC)"; \
	fi
	@echo ""

# Test API connectivity
test:
	@echo "$(BLUE)Testing API Connectivity$(NC)"
	@echo "$(BLUE)═══════════════════════════════════════════════════════════$(NC)"
	@echo ""
	@if [ ! -f "secrets.py" ]; then \
		echo "$(YELLOW)Note: secrets.py not found. Using example values.$(NC)"; \
		echo "Run 'make config' first to set up actual credentials.$(NC)"; \
		echo ""; \
		API_BASE_URL="https://matrix-miles-production.up.railway.app"; \
		API_KEY="test-key"; \
	else \
		API_BASE_URL=$$(grep "API_BASE_URL" secrets.py | cut -d'"' -f2); \
		API_KEY=$$(grep "API_KEY" secrets.py | cut -d'"' -f2); \
	fi
	@echo "$(YELLOW)Testing API base URL: $$API_BASE_URL$(NC)"
	@echo ""
	@echo "$(BLUE)Health check:$(NC)"
	@curl -s -I "$$API_BASE_URL/health" | head -3 || echo "$(RED)Failed to connect$(NC)"
	@echo ""
	@echo "$(BLUE)API test (this requires valid API key):$(NC)"
	@echo "  Run: make test-api API_KEY=your-key USER_ID=1"
	@echo ""

test-api:
	@if [ -z "$(API_KEY)" ] || [ -z "$(USER_ID)" ]; then \
		echo "$(RED)Usage: make test-api API_KEY=<key> USER_ID=<id>$(NC)"; \
		exit 1; \
	fi
	@echo "$(BLUE)Testing API request...$(NC)"
	@API_BASE_URL=$$(grep "API_BASE_URL" secrets.py 2>/dev/null | cut -d'"' -f2 || echo "https://matrix-miles-production.up.railway.app"); \
	echo "Base URL: $$API_BASE_URL"; \
	echo "User ID: $(USER_ID)"; \
	echo ""; \
	echo "Fetching activities..."; \
	curl -s -H "X-API-Key: $(API_KEY)" \
	     "$$API_BASE_URL/api/activities/recent/$(USER_ID)" | python3 -m json.tool

# Clean local temporary files
clean:
	@echo "$(BLUE)Cleaning up...$(NC)"
	@find . -type f -name "*.pyc" -delete
	@find . -type d -name "__pycache__" -delete
	@find . -type f -name "*.swp" -delete
	@find . -type f -name "*~" -delete
	@echo "$(GREEN)✓ Cleaned$(NC)"

# Internal target to check mount
check-mount:
	@if [ -z "$(MOUNT)" ]; then \
		echo "$(RED)Error: CIRCUITPY not mounted$(NC)"; \
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
	@echo "$(BLUE)Platform Detection$(NC)"
	@echo "OS: $$(uname -s)"
	@echo "Mount: $(MOUNT)"
	@echo "Device: $(DEVICE_PORT)$(DEVICE_PORT_ALT)"
