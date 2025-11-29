"""
WiFi Manager Module

Handles WiFi connection and reconnection logic for MatrixPortal M4.
Provides blocking and non-blocking connection methods.
"""

import time
import wifi
import socketpool


class WiFiManager:
    """Manages WiFi connections with automatic reconnection"""

    def __init__(self, ssid, password):
        """
        Initialize WiFi manager

        Args:
            ssid (str): WiFi network name
            password (str): WiFi password
        """
        self.ssid = ssid
        self.password = password
        self.connected = False
        self.last_connection_attempt = 0
        self.connection_retry_interval = 10  # seconds

    def connect(self, timeout=30):
        """
        Connect to WiFi with timeout

        Args:
            timeout (int): Connection timeout in seconds

        Returns:
            bool: True if connected, False otherwise
        """
        print(f"[WiFiManager] Connecting to {self.ssid}...")
        start_time = time.monotonic()

        try:
            # Try to connect
            wifi.radio.connect(self.ssid, self.password)

            # Wait for connection with timeout
            while not wifi.radio.connected and (
                time.monotonic() - start_time < timeout
            ):
                time.sleep(0.5)

            if wifi.radio.connected:
                self.connected = True
                print("[WiFiManager] Connected!")
                return True
            else:
                print("[WiFiManager] Connection timeout")
                self.connected = False
                return False

        except Exception as e:
            print(f"[WiFiManager] Connection error: {e}")
            self.connected = False
            return False

    def is_connected(self):
        """
        Check if WiFi is currently connected

        Returns:
            bool: True if connected, False otherwise
        """
        try:
            return wifi.radio.connected
        except Exception:
            return False

    def get_ip(self):
        """
        Get current IP address

        Returns:
            str: IP address or "Not connected" if not connected
        """
        try:
            if wifi.radio.connected:
                ip = wifi.radio.ipv4_address
                return str(ip)
            else:
                return "Not connected"
        except Exception as e:
            print(f"[WiFiManager] Error getting IP: {e}")
            return "Error"

    def get_signal_strength(self):
        """
        Get WiFi signal strength

        Returns:
            int: RSSI value (negative, -30 is strong, -90 is weak)
        """
        try:
            return wifi.radio.ap_info.rssi
        except Exception:
            return 0

    def disconnect(self):
        """Disconnect from WiFi"""
        try:
            wifi.radio.stop_station()
            self.connected = False
            print("[WiFiManager] Disconnected")
            return True
        except Exception as e:
            print(f"[WiFiManager] Disconnect error: {e}")
            return False

    def reconnect_if_needed(self):
        """
        Attempt to reconnect if disconnected

        Returns:
            bool: True if connected (or was already connected), False if reconnect failed
        """
        if self.is_connected():
            return True

        current_time = time.monotonic()
        if current_time - self.last_connection_attempt < self.connection_retry_interval:
            return False

        self.last_connection_attempt = current_time
        return self.connect(timeout=15)
