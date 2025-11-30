"""
Main application entry point for MatrixPortal M4 CircuitPython

Fetches Strava activity data from the Matrix Miles backend via HTTPS
and displays it as a calendar on the LED matrix.

Flow:
1. Initialize display and serial logging
2. Connect to WiFi (with auto-reconnect)
3. Fetch activity data from backend API
4. Parse and render calendar view
5. Sleep and repeat
"""

import time
import sys
import board
import displayio
import supervisor
import busio
from digitalio import DigitalInOut
from os import getenv

# Add lib directory to path for imports
sys.path.insert(0, "/lib")

from calendar_display import CalendarDisplay

# Import ESP32 SPI driver for MatrixPortal M4
from adafruit_esp32spi import adafruit_esp32spi
import adafruit_connection_manager
import adafruit_requests

# Get WiFi settings from settings.toml
WIFI_SSID = getenv("CIRCUITPY_WIFI_SSID")
WIFI_PASSWORD = getenv("CIRCUITPY_WIFI_PASSWORD")

# Get API settings from secrets or settings.toml
API_BASE_URL = getenv("API_BASE_URL", "http://localhost:3000")
API_KEY = getenv("API_KEY", "")
USER_ID = int(getenv("USER_ID", "1"))
REFRESH_INTERVAL_SECONDS = int(getenv("REFRESH_INTERVAL_SECONDS", "300"))

# ============================================================================
# CONFIGURATION
# ============================================================================

DEBUG = True  # Set to False for production (reduces serial output)
DISPLAY_BRIGHTNESS = 1.0

# ============================================================================
# ESP32 SPI SETUP (MatrixPortal M4)
# ============================================================================


def init_esp32_spi():
    """Initialize ESP32 co-processor via SPI"""
    log("Initializing ESP32 SPI...")

    # MatrixPortal M4 has predefined ESP32 pins
    esp32_cs = DigitalInOut(board.ESP_CS)
    esp32_ready = DigitalInOut(board.ESP_BUSY)
    esp32_reset = DigitalInOut(board.ESP_RESET)

    spi = busio.SPI(board.SCK, board.MOSI, board.MISO)
    esp = adafruit_esp32spi.ESP_SPIcontrol(spi, esp32_cs, esp32_ready, esp32_reset)

    if esp.status == adafruit_esp32spi.WL_IDLE_STATUS:
        log("ESP32 found and in idle mode")
    log(f"ESP32 Firmware: {esp.firmware_version}")

    return esp


def get_http_session(esp):
    """Create HTTP session with connection pooling"""
    pool = adafruit_connection_manager.get_radio_socketpool(esp)
    ssl_context = adafruit_connection_manager.get_radio_ssl_context(esp)
    return adafruit_requests.Session(pool, ssl_context)


# ============================================================================
# HELPERS
# ============================================================================


def log(message, level="INFO"):
    """Structured logging to serial"""
    if DEBUG or level in ("ERROR", "WARNING"):
        timestamp = time.monotonic()
        print(f"[{timestamp:.1f}] [{level}] {message}")


def log_dict(label, data):
    """Log a dictionary nicely"""
    log(f"{label}:")
    for key, value in data.items():
        log(f"  {key}: {value}")


# ============================================================================
# MAIN APPLICATION
# ============================================================================


class MatrixMilesApp:
    """Main application controller"""

    def __init__(self):
        """Initialize application components"""
        log("=== Matrix Miles (CircuitPython) ===")

        # Initialize display first
        self.display = CalendarDisplay()
        self.display.show_message("Initializing...")
        log("Display initialized")

        # Initialize ESP32 SPI
        try:
            self.esp = init_esp32_spi()
            log("ESP32 SPI initialized")
        except Exception as e:
            log(f"ESP32 initialization error: {e}", level="ERROR")
            self.display.show_message("ESP32 Error", duration=2)
            raise

        # Initialize HTTP session
        try:
            self.session = get_http_session(self.esp)
            log("HTTP session created")
        except Exception as e:
            log(f"HTTP session error: {e}", level="ERROR")
            self.display.show_message("HTTP Error", duration=2)
            raise

        self.connected = False
        self.last_fetch = 0
        self.fetch_count = 0
        self.error_count = 0

    def connect_wifi(self):
        """Connect to WiFi with retry logic"""
        log("Connecting to WiFi...")
        self.display.show_message("WiFi: Connecting...")

        try:
            while not self.esp.is_connected:
                try:
                    log(f"Attempting to connect to {WIFI_SSID}...")
                    self.esp.connect_AP(WIFI_SSID, WIFI_PASSWORD)
                except OSError as e:
                    log(f"WiFi connection error: {e}", level="WARNING")
                    time.sleep(1)
                    continue

            ip = self.esp.ipv4_address
            log(f"WiFi connected: {ip}")
            self.display.show_message(f"WiFi: {ip}", duration=3)
            self.connected = True
            return True

        except Exception as e:
            log(f"WiFi connection failed: {e}", level="ERROR")
            self.display.show_message("WiFi: Failed", duration=2)
            self.error_count += 1
            self.connected = False
            return False

    def fetch_activities(self):
        """Fetch activity data from backend"""
        log("Fetching activities...")
        self.display.show_message("Fetching...")

        try:
            endpoint = f"/api/activities/recent/{USER_ID}"
            url = f"{API_BASE_URL}{endpoint}"

            log(f"GET {url}")

            headers = {
                "User-Agent": "MatrixPortal-CircuitPython/1.0",
                "X-API-Key": API_KEY,
            }

            response = self.session.get(url, headers=headers, timeout=10)
            log(f"Status: {response.status_code}")

            if response.status_code != 200:
                log(f"HTTP error {response.status_code}", level="ERROR")
                response.close()
                return None

            data = response.json()
            response.close()

            if isinstance(data, list):
                log(f"API response received: {len(data)} activities")
                self.fetch_count += 1
                return data
            else:
                log(f"Unexpected response format: {type(data)}", level="ERROR")
                return None

        except Exception as e:
            log(f"Error fetching activities: {e}", level="ERROR")
            self.error_count += 1
            return None

    def render_calendar(self, activities):
        """Render activities as calendar on display"""
        if activities is None or len(activities) == 0:
            log("No activities to render")
            self.display.show_message("No activities", duration=2)
            return

        log(f"Rendering {len(activities)} activities")
        try:
            self.display.render_calendar(activities)
            log("Calendar rendered successfully")
        except Exception as e:
            log(f"Error rendering calendar: {e}", level="ERROR")
            self.display.show_message("Render Error", duration=2)

    def update(self):
        """Main update loop"""
        current_time = time.monotonic()

        # Check WiFi connection periodically
        if not self.esp.is_connected:
            log("WiFi disconnected, reconnecting...")
            if not self.connect_wifi():
                self.display.show_message("No WiFi", duration=1)
                return

        # Fetch new data on interval
        if current_time - self.last_fetch >= REFRESH_INTERVAL_SECONDS:
            activities = self.fetch_activities()
            if activities is not None:
                self.render_calendar(activities)
            self.last_fetch = current_time

    def run(self):
        """Main application loop"""
        log("Starting main loop...")
        self.display.show_message("Starting...")

        # Initial WiFi connection
        if not self.connect_wifi():
            log("Initial WiFi connection failed, will retry", level="WARNING")

        # Initial fetch
        activities = self.fetch_activities()
        if activities is not None:
            self.render_calendar(activities)

        # Main loop
        loop_count = 0
        while True:
            try:
                loop_count += 1
                self.update()

                # Log status periodically
                if loop_count % 100 == 0:
                    log(
                        f"Status: fetches={self.fetch_count}, errors={self.error_count}"
                    )

                time.sleep(1)  # Update loop runs 1x per second

            except Exception as e:
                log(f"Unhandled error in main loop: {e}", level="ERROR")
                self.display.show_message("ERROR", duration=2)
                self.error_count += 1
                time.sleep(5)  # Back off on error


# ============================================================================
# ENTRY POINT
# ============================================================================

if __name__ == "__main__":
    app = MatrixMilesApp()
    app.run()
