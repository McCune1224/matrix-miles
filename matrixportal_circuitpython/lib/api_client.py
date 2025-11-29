"""
API Client Module

Handles HTTPS requests to the Matrix Miles backend with error handling
and JSON parsing. Uses CircuitPython's built-in requests library.
"""

import json
import time
import adafruit_requests
import ssl
import socketpool
import wifi


class APIClient:
    """Client for communicating with Matrix Miles backend"""

    def __init__(self, base_url, api_key, user_id, timeout=10):
        """
        Initialize API client

        Args:
            base_url (str): Base URL of the API (e.g., https://...)
            api_key (str): API key for authentication
            user_id (int): User ID for activity requests
            timeout (int): Request timeout in seconds
        """
        self.base_url = base_url
        self.api_key = api_key
        self.user_id = user_id
        self.timeout = timeout

        # Create HTTPS session with SSL
        self.pool = socketpool.SocketPool(wifi.radio)
        self.session = adafruit_requests.Session(
            self.pool, ssl_context=ssl.create_default_context()
        )

        # Default headers
        self.headers = {
            "User-Agent": "MatrixPortal-CircuitPython/1.0",
            "Content-Type": "application/json",
            "X-API-Key": self.api_key,
        }

    def _make_request(self, endpoint, method="GET", timeout=None):
        """
        Make an HTTP request to the API

        Args:
            endpoint (str): API endpoint path (e.g., "/api/activities/recent/1")
            method (str): HTTP method (GET, POST, etc.)
            timeout (int): Request timeout (uses self.timeout if not specified)

        Returns:
            dict: Parsed JSON response, or None on error
        """
        if timeout is None:
            timeout = self.timeout

        url = f"{self.base_url}{endpoint}"
        print(f"[APIClient] {method} {url}")

        try:
            if method == "GET":
                response = self.session.get(url, headers=self.headers, timeout=timeout)
            else:
                print(f"[APIClient] Unsupported method: {method}")
                return None

            # Log response
            print(f"[APIClient] Status: {response.status_code}")

            # Check for errors
            if response.status_code not in (200, 201):
                print(f"[APIClient] HTTP error {response.status_code}: {response.text}")
                response.close()
                return None

            # Parse JSON
            try:
                data = response.json()
                print(f"[APIClient] Response OK")
                response.close()
                return data
            except Exception as e:
                print(f"[APIClient] JSON parse error: {e}")
                print(f"[APIClient] Response text: {response.text[:200]}")
                response.close()
                return None

        except Exception as e:
            print(f"[APIClient] Request error: {e}")
            return None

    def get_recent_activities(self):
        """
        Fetch recent activities for the user

        Returns:
            list: List of activity dictionaries, or None on error

        Example response:
            [
                {
                    "id": 1,
                    "name": "Morning Run",
                    "activity_date": "2024-01-15",
                    "type": "run",
                    "distance": 5.2
                },
                ...
            ]
        """
        endpoint = f"/api/activities/recent/{self.user_id}"
        response = self._make_request(endpoint)

        if response is None:
            return None

        # Ensure response is a list
        if isinstance(response, list):
            return response
        else:
            print(f"[APIClient] Unexpected response format: {type(response)}")
            return None

    def get_calendar_data(self, year, month):
        """
        Fetch calendar data for a specific month

        Args:
            year (int): Year (e.g., 2024)
            month (int): Month (1-12)

        Returns:
            dict: Calendar data with activity dates and stats, or None on error

        Example response:
            {
                "year": 2024,
                "month": 1,
                "days_with_activities": [1, 3, 5, 8, 10, ...],
                "total_activities": 15,
                "total_distance": 52.3
            }
        """
        endpoint = f"/api/activities/calendar/{self.user_id}/{year}/{month}"
        return self._make_request(endpoint)

    def get_user_stats(self):
        """
        Fetch overall user statistics

        Returns:
            dict: User stats including totals, or None on error
        """
        endpoint = f"/api/users/{self.user_id}"
        return self._make_request(endpoint)

    def health_check(self):
        """
        Check if the API is reachable

        Returns:
            bool: True if API is responding, False otherwise
        """
        try:
            response = self.session.get(f"{self.base_url}/health", timeout=self.timeout)
            is_healthy = response.status_code == 200
            response.close()
            return is_healthy
        except Exception as e:
            print(f"[APIClient] Health check failed: {e}")
            return False
