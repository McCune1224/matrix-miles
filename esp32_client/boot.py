# boot.py - Runs on ESP32 startup

import network
import time
from esp32_client.config import WIFI_SSID, WIFI_PASSWORD


def connect_wifi():
    """Connect to WiFi network"""
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)

    if wlan.isconnected():
        print("Already connected to WiFi")
        print("IP:", wlan.ifconfig()[0])
        return True

    print("Connecting to WiFi:", WIFI_SSID)
    wlan.connect(WIFI_SSID, WIFI_PASSWORD)

    # Wait up to 15 seconds for connection
    timeout = 15
    start = time.time()

    while not wlan.isconnected():
        if time.time() - start > timeout:
            print("ERROR: WiFi connection timeout!")
            print("Check SSID and password in config.py")
            return False

        print(".", end="")
        time.sleep(0.5)

    print("\nWiFi connected!")
    print("IP address:", wlan.ifconfig()[0])
    print("Gateway:", wlan.ifconfig()[2])
    return True


# Auto-connect on boot
print("=== ESP32 Strava Client Starting ===")
if connect_wifi():
    print("Ready to run main.py")
else:
    print("WiFi connection failed!")
