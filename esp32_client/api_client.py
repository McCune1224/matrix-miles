# api_client.py - HTTP client for Strava server

try:
    import urequests as requests
except ImportError:
    import requests  # Fallback for testing

import json
from esp32_client.config import API_BASE_URL, API_KEY, USER_ID


class StravaClient:
    """Client for communicating with Go server"""

    def __init__(self):
        self.base_url: str = API_BASE_URL.rstrip("/")
        self.user_id: int = USER_ID
        self.headers: dict[str,str] = {"X-API-Key": API_KEY, "Content-Type": "application/json"}

    def health_check(self):
        """Check if server is reachable"""
        try:
            url = f"{self.base_url}/health"
            print(f"Checking: {url}")
            response = requests.get(url, timeout=5)
            success = response.status_code == 200
            response.close()
            return success
        except Exception as e:
            print(f"Health check failed: {e}")
            return False

    def get_recent_activities(self, limit=5):
        """Fetch recent activities from server"""
        try:
            url = f"{self.base_url}/api/activities/recent/{self.user_id}"
            print(f"Fetching: {url}")

            response = requests.get(url, headers=self.headers, timeout=10)

            if response.status_code == 200:
                data = response.json()
                response.close()
                return data[:limit] if data else []
            elif response.status_code == 401:
                print("ERROR: Invalid API key")
                response.close()
                return None
            else:
                print(f"ERROR: Server returned {response.status_code}")
                response.close()
                return None

        except Exception as e:
            print(f"Request failed: {e}")
            return None

    def get_stats(self):
        """Fetch user statistics"""
        try:
            url = f"{self.base_url}/api/stats/{self.user_id}"
            print(f"Fetching: {url}")

            response = requests.get(url, headers=self.headers, timeout=10)

            if response.status_code == 200:
                data = response.json()
                response.close()
                return data
            else:
                print(f"ERROR: Server returned {response.status_code}")
                response.close()
                return None

        except Exception as e:
            print(f"Request failed: {e}")
            return None

    def get_calendar(self, year, month):
        """Fetch calendar data for a specific month"""
        try:
            url = (
                f"{self.base_url}/api/activities/calendar/{self.user_id}/{year}/{month}"
            )
            print(f"Fetching: {url}")

            response = requests.get(url, headers=self.headers, timeout=10)

            if response.status_code == 200:
                data = response.json()
                response.close()
                return data
            else:
                print(f"ERROR: Server returned {response.status_code}")
                response.close()
                return None

        except Exception as e:
            print(f"Request failed: {e}")
            return None
