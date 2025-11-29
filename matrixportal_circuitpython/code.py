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
import board
import displayio
import supervisor
from lib.wifi_manager import WiFiManager
from lib.api_client import APIClient
from lib.calendar_display import CalendarDisplay

try:
    from secrets import (
        WIFI_SSID,
        WIFI_PASSWORD,
        API_BASE_URL,
        API_KEY,
        USER_ID,
        REFRESH_INTERVAL_SECONDS,
    )
except ImportError:
    print("ERROR: secrets.py not found!")
    print("Copy config.example.py to secrets.py and configure it.")
    supervisor.reload()

# ============================================================================
# CONFIGURATION
# ============================================================================

DEBUG = True  # Set to False for production (reduces serial output)
DISPLAY_BRIGHTNESS = 1.0

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

        # Initialize WiFi manager
        self.wifi = WiFiManager(ssid=WIFI_SSID, password=WIFI_PASSWORD)
        log("WiFi manager created")

        # Initialize API client
        self.api_client = APIClient(
            base_url=API_BASE_URL, api_key=API_KEY, user_id=USER_ID
        )
        log("API client created")

        self.last_fetch = 0
        self.fetch_count = 0
        self.error_count = 0

    def connect_wifi(self):
        """Connect to WiFi with retry logic"""
        log("Connecting to WiFi...")
        self.display.show_message("WiFi: Connecting...")

        if self.wifi.connect(timeout=30):
            ip = self.wifi.get_ip()
            log(f"WiFi connected: {ip}")
            self.display.show_message(f"WiFi: {ip}", duration=3)
            return True
        else:
            log("WiFi connection failed", level="ERROR")
            self.display.show_message("WiFi: Failed", duration=2)
            self.error_count += 1
            return False

    def fetch_activities(self):
        """Fetch activity data from backend"""
        log("Fetching activities...")
        self.display.show_message("Fetching...")

        try:
            # Make API request
            response = self.api_client.get_recent_activities()

            if response is None:
                log("API request returned None", level="ERROR")
                self.error_count += 1
                return None

            log(f"API response received: {len(response)} activities")
            self.fetch_count += 1

            return response

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
        if not self.wifi.is_connected():
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
