# Matrix Miles - API Endpoints Reference

This document describes the backend API endpoints that the CircuitPython client uses.

## Base URL

- **Production:** `https://matrix-miles-production.up.railway.app`
- **Local Development:** `http://localhost:8080` (if running locally)

## Authentication

All requests require the `X-API-Key` header:

```
X-API-Key: your-api-key-here
```

Example:
```bash
curl -H "X-API-Key: abc123xyz" \
     https://matrix-miles-production.up.railway.app/api/activities/recent/1
```

## Endpoints

### 1. Get Recent Activities

**Endpoint:** `GET /api/activities/recent/{USER_ID}`

Fetches the most recent activities for a user.

**Parameters:**
- `USER_ID` (path): User ID (integer)

**Response (200 OK):**
```json
[
  {
    "id": 1,
    "user_id": 1,
    "strava_activity_id": 12345,
    "name": "Morning Run",
    "activity_date": "2024-01-15",
    "type": "run",
    "distance": 5.2,
    "elapsed_time": 1800,
    "created_at": "2024-01-15T08:30:00Z",
    "updated_at": "2024-01-15T08:30:00Z"
  },
  {
    "id": 2,
    "user_id": 1,
    "strava_activity_id": 12346,
    "name": "Evening Bike Ride",
    "activity_date": "2024-01-14",
    "type": "ride",
    "distance": 15.3,
    "elapsed_time": 3600,
    "created_at": "2024-01-14T18:45:00Z",
    "updated_at": "2024-01-14T18:45:00Z"
  }
]
```

**Error Responses:**
- `401 Unauthorized` - Invalid or missing API key
- `404 Not Found` - User ID doesn't exist
- `500 Internal Server Error` - Server error

### 2. Get Calendar Data

**Endpoint:** `GET /api/activities/calendar/{USER_ID}/{YEAR}/{MONTH}`

Fetches calendar data for a specific month (all activities for that month).

**Parameters:**
- `USER_ID` (path): User ID (integer)
- `YEAR` (path): Year (e.g., 2024)
- `MONTH` (path): Month (1-12)

**Response (200 OK):**
```json
[
  {
    "id": 5,
    "user_id": 1,
    "strava_activity_id": 12350,
    "name": "Run",
    "activity_date": "2024-01-01",
    "type": "run",
    "distance": 3.0,
    "elapsed_time": 900,
    "created_at": "2024-01-01T07:00:00Z",
    "updated_at": "2024-01-01T07:00:00Z"
  },
  {
    "id": 6,
    "user_id": 1,
    "strava_activity_id": 12351,
    "name": "Run",
    "activity_date": "2024-01-03",
    "type": "run",
    "distance": 5.5,
    "elapsed_time": 1650,
    "created_at": "2024-01-03T08:15:00Z",
    "updated_at": "2024-01-03T08:15:00Z"
  }
]
```

### 3. Get User Statistics

**Endpoint:** `GET /api/users/{USER_ID}`

Fetches overall statistics for a user.

**Parameters:**
- `USER_ID` (path): User ID (integer)

**Response (200 OK):**
```json
{
  "id": 1,
  "strava_id": 67890,
  "email": "user@example.com",
  "first_name": "John",
  "last_name": "Doe",
  "created_at": "2024-01-01T00:00:00Z",
  "updated_at": "2024-01-15T10:00:00Z"
}
```

### 4. Health Check

**Endpoint:** `GET /health`

Simple health check endpoint (no auth required).

**Response (200 OK):**
```json
{
  "status": "ok"
}
```

## Usage in CircuitPython

```python
from lib.api_client import APIClient

# Create client
client = APIClient(
    base_url="https://matrix-miles-production.up.railway.app",
    api_key="your-api-key",
    user_id=1
)

# Get recent activities
activities = client.get_recent_activities()
# Returns: list of activity dicts or None on error

# Get calendar data for January 2024
calendar = client.get_calendar_data(year=2024, month=1)
# Returns: list of activities for that month or None on error

# Get user stats
stats = client.get_user_stats()
# Returns: user dict or None on error

# Health check
is_alive = client.health_check()
# Returns: True if API is responding
```

## Activity Types

The `type` field can be one of:
- `run` - Running activity
- `ride` - Cycling activity
- `swim` - Swimming activity
- `walk` - Walking activity
- `hike` - Hiking activity
- `other` - Other activity types

## Date Format

All dates in responses use ISO 8601 format:
- `activity_date`: `YYYY-MM-DD` (date only)
- `created_at`, `updated_at`: `YYYY-MM-DDTHH:MM:SSZ` (timestamp)

## Error Handling

Always check the response before using:

```python
activities = client.get_recent_activities()

if activities is None:
    print("Failed to fetch activities")
    # Handle error (retry, display message, etc.)
else:
    print(f"Got {len(activities)} activities")
    for activity in activities:
        print(f"  - {activity['name']} on {activity['activity_date']}")
```

## Rate Limiting

Currently no rate limiting is implemented, but be respectful:
- Don't fetch more often than every 5 minutes
- Consider using longer intervals (10-30 minutes) for production

## CORS

The API allows CORS requests from any origin (configured for IoT devices).

## SSL/TLS

The production API requires HTTPS. The CircuitPython client handles SSL certificate verification automatically.

For development with self-signed certificates, see the GETTING_STARTED.md troubleshooting section.
