# ESP32 Strava Activity Display - Complete Setup Guide
## Architecture: Go Backend + MicroPython ESP32 Client

This guide sets up a complete system with a Go HTTP server (using Echo) that handles OAuth, Strava API calls, and PostgreSQL storage, while the ESP32 acts as a simple display client.

---

## Architecture Overview

```
┌─────────────────┐
│  User Browser   │ ← OAuth Authorization
└────────┬────────┘
         │
         ↓
┌─────────────────────────────────┐
│  Go HTTP Server (Echo)          │
│  - OAuth flow handler           │
│  - Strava API client            │
│  - Token management/refresh     │
│  - PostgreSQL integration       │
│  - REST API for ESP32           │
└────────┬────────────────────────┘
         │
         ↓
┌─────────────────┐
│  PostgreSQL DB  │
└─────────────────┘
         ↑
         │
┌────────┴────────┐
│  ESP32 Client   │ ← Simple HTTP requests
│  - WiFi         │
│  - LED Matrix   │
└─────────────────┘
```

**Benefits:**
- ESP32 has minimal code (just HTTP client)
- All complex logic in Go server
- Easy to test and debug
- Scalable for multiple devices
- Centralized token management

---

## Part 1: Go HTTP Server Setup

### Prerequisites
- Go 1.21+ installed
- PostgreSQL database (local or cloud)
- Domain name (optional but recommended for OAuth)
- Strava API credentials

### Step 1.1: Install Go

```bash
# Linux
wget https://go.dev/dl/go1.21.5.linux-amd64.tar.gz
sudo tar -C /usr/local -xzf go1.21.5.linux-amd64.tar.gz
echo 'export PATH=$PATH:/usr/local/go/bin' >> ~/.bashrc
source ~/.bashrc
go version
```

### Step 1.2: Create Go Project

```bash
mkdir strava-server
cd strava-server
go mod init github.com/yourusername/strava-server

# Install dependencies
go get github.com/labstack/echo/v4
go get github.com/labstack/echo/v4/middleware
go get github.com/lib/pq
go get github.com/joho/godotenv
```

### Step 1.3: Project Structure

```
strava-server/
├── main.go
├── handlers/
│   ├── oauth.go
│   ├── strava.go
│   └── esp32.go
├── models/
│   └── activity.go
├── db/
│   └── postgres.go
├── strava/
│   └── client.go
├── .env
├── go.mod
└── go.sum
```

### Step 1.4: Environment Configuration

Create `.env`:
```env
# Server
PORT=8080
DOMAIN=https://yourdomain.com

# Strava API
STRAVA_CLIENT_ID=your_client_id
STRAVA_CLIENT_SECRET=your_client_secret
STRAVA_REDIRECT_URI=https://yourdomain.com/auth/callback

# Database
DB_HOST=localhost
DB_PORT=5432
DB_USER=your_db_user
DB_PASSWORD=your_db_password
DB_NAME=strava_db

# API Key for ESP32 (generate a secure random string)
ESP32_API_KEY=your_secure_api_key_here
```

### Step 1.5: Database Schema

Create `schema.sql`:
```sql
-- Users table
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    strava_user_id BIGINT UNIQUE NOT NULL,
    access_token TEXT NOT NULL,
    refresh_token TEXT NOT NULL,
    token_expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Activities table
CREATE TABLE activities (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id),
    strava_activity_id BIGINT UNIQUE NOT NULL,
    name TEXT,
    type TEXT,
    distance FLOAT,
    moving_time INTEGER,
    start_date TIMESTAMP,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Create indexes
CREATE INDEX idx_activities_user_id ON activities(user_id);
CREATE INDEX idx_activities_start_date ON activities(start_date);
CREATE INDEX idx_users_strava_id ON users(strava_user_id);
```

Apply schema:
```bash
psql -U your_db_user -d strava_db -f schema.sql
```

### Step 1.6: Go Server Implementation

**`main.go`:**
```go
package main

import (
    "log"
    "os"
    
    "github.com/joho/godotenv"
    "github.com/labstack/echo/v4"
    "github.com/labstack/echo/v4/middleware"
    "github.com/yourusername/strava-server/db"
    "github.com/yourusername/strava-server/handlers"
)

func main() {
    // Load environment variables
    if err := godotenv.Load(); err != nil {
        log.Println("No .env file found")
    }
    
    // Initialize database
    database, err := db.NewPostgresDB()
    if err != nil {
        log.Fatal("Failed to connect to database:", err)
    }
    defer database.Close()
    
    // Initialize Echo
    e := echo.New()
    
    // Middleware
    e.Use(middleware.Logger())
    e.Use(middleware.Recover())
    e.Use(middleware.CORS())
    
    // Initialize handlers
    oauthHandler := handlers.NewOAuthHandler(database)
    stravaHandler := handlers.NewStravaHandler(database)
    esp32Handler := handlers.NewESP32Handler(database)
    
    // Routes
    
    // OAuth routes (for web browser)
    e.GET("/auth/login", oauthHandler.InitiateLogin)
    e.GET("/auth/callback", oauthHandler.Callback)
    
    // Admin routes (protected)
    admin := e.Group("/admin")
    admin.Use(middleware.BasicAuth(func(username, password string, c echo.Context) (bool, error) {
        // Simple auth - improve for production
        return username == "admin" && password == os.Getenv("ADMIN_PASSWORD"), nil
    }))
    admin.GET("/users", stravaHandler.ListUsers)
    admin.POST("/sync/:userId", stravaHandler.SyncActivities)
    
    // ESP32 API routes (protected with API key)
    api := e.Group("/api")
    api.Use(apiKeyMiddleware)
    api.GET("/activities/recent/:userId", esp32Handler.GetRecentActivities)
    api.GET("/activities/calendar/:userId/:year/:month", esp32Handler.GetCalendarData)
    api.GET("/stats/:userId", esp32Handler.GetUserStats)
    
    // Health check
    e.GET("/health", func(c echo.Context) error {
        return c.JSON(200, map[string]string{"status": "ok"})
    })
    
    // Start server
    port := os.Getenv("PORT")
    if port == "" {
        port = "8080"
    }
    
    log.Printf("Server starting on port %s", port)
    e.Logger.Fatal(e.Start(":" + port))
}

// API Key middleware for ESP32 authentication
func apiKeyMiddleware(next echo.HandlerFunc) echo.HandlerFunc {
    return func(c echo.Context) error {
        apiKey := c.Request().Header.Get("X-API-Key")
        expectedKey := os.Getenv("ESP32_API_KEY")
        
        if apiKey == "" || apiKey != expectedKey {
            return echo.NewHTTPError(401, "Invalid API key")
        }
        
        return next(c)
    }
}
```

**`db/postgres.go`:**
```go
package db

import (
    "database/sql"
    "fmt"
    "os"
    
    _ "github.com/lib/pq"
)

type PostgresDB struct {
    *sql.DB
}

func NewPostgresDB() (*PostgresDB, error) {
    connStr := fmt.Sprintf(
        "host=%s port=%s user=%s password=%s dbname=%s sslmode=disable",
        os.Getenv("DB_HOST"),
        os.Getenv("DB_PORT"),
        os.Getenv("DB_USER"),
        os.Getenv("DB_PASSWORD"),
        os.Getenv("DB_NAME"),
    )
    
    db, err := sql.Open("postgres", connStr)
    if err != nil {
        return nil, err
    }
    
    if err := db.Ping(); err != nil {
        return nil, err
    }
    
    return &PostgresDB{db}, nil
}
```

**`models/activity.go`:**
```go
package models

import "time"

type User struct {
    ID             int       `json:"id"`
    StravaUserID   int64     `json:"strava_user_id"`
    AccessToken    string    `json:"-"`
    RefreshToken   string    `json:"-"`
    TokenExpiresAt time.Time `json:"-"`
    CreatedAt      time.Time `json:"created_at"`
    UpdatedAt      time.Time `json:"updated_at"`
}

type Activity struct {
    ID               int       `json:"id"`
    UserID           int       `json:"user_id"`
    StravaActivityID int64     `json:"strava_activity_id"`
    Name             string    `json:"name"`
    Type             string    `json:"type"`
    Distance         float64   `json:"distance"`
    MovingTime       int       `json:"moving_time"`
    StartDate        time.Time `json:"start_date"`
    CreatedAt        time.Time `json:"created_at"`
}

type CalendarDay struct {
    Date        string  `json:"date"`
    HasActivity bool    `json:"has_activity"`
    Count       int     `json:"count"`
    Distance    float64 `json:"total_distance"`
}
```

**`strava/client.go`:**
```go
package strava

import (
    "encoding/json"
    "fmt"
    "io"
    "net/http"
    "net/url"
    "os"
    "strings"
    "time"
)

type Client struct {
    httpClient *http.Client
}

type TokenResponse struct {
    AccessToken  string `json:"access_token"`
    RefreshToken string `json:"refresh_token"`
    ExpiresAt    int64  `json:"expires_at"`
    Athlete      struct {
        ID int64 `json:"id"`
    } `json:"athlete"`
}

type StravaActivity struct {
    ID          int64     `json:"id"`
    Name        string    `json:"name"`
    Type        string    `json:"type"`
    Distance    float64   `json:"distance"`
    MovingTime  int       `json:"moving_time"`
    StartDate   time.Time `json:"start_date"`
}

func NewClient() *Client {
    return &Client{
        httpClient: &http.Client{Timeout: 10 * time.Second},
    }
}

func (c *Client) ExchangeCode(code string) (*TokenResponse, error) {
    data := url.Values{}
    data.Set("client_id", os.Getenv("STRAVA_CLIENT_ID"))
    data.Set("client_secret", os.Getenv("STRAVA_CLIENT_SECRET"))
    data.Set("code", code)
    data.Set("grant_type", "authorization_code")
    
    resp, err := c.httpClient.Post(
        "https://www.strava.com/oauth/token",
        "application/x-www-form-urlencoded",
        strings.NewReader(data.Encode()),
    )
    if err != nil {
        return nil, err
    }
    defer resp.Body.Close()
    
    if resp.StatusCode != http.StatusOK {
        body, _ := io.ReadAll(resp.Body)
        return nil, fmt.Errorf("strava API error: %s", string(body))
    }
    
    var tokenResp TokenResponse
    if err := json.NewDecoder(resp.Body).Decode(&tokenResp); err != nil {
        return nil, err
    }
    
    return &tokenResp, nil
}

func (c *Client) RefreshToken(refreshToken string) (*TokenResponse, error) {
    data := url.Values{}
    data.Set("client_id", os.Getenv("STRAVA_CLIENT_ID"))
    data.Set("client_secret", os.Getenv("STRAVA_CLIENT_SECRET"))
    data.Set("refresh_token", refreshToken)
    data.Set("grant_type", "refresh_token")
    
    resp, err := c.httpClient.Post(
        "https://www.strava.com/oauth/token",
        "application/x-www-form-urlencoded",
        strings.NewReader(data.Encode()),
    )
    if err != nil {
        return nil, err
    }
    defer resp.Body.Close()
    
    var tokenResp TokenResponse
    if err := json.NewDecoder(resp.Body).Decode(&tokenResp); err != nil {
        return nil, err
    }
    
    return &tokenResp, nil
}

func (c *Client) GetActivities(accessToken string, after int64, perPage int) ([]StravaActivity, error) {
    url := fmt.Sprintf(
        "https://www.strava.com/api/v3/athlete/activities?after=%d&per_page=%d",
        after, perPage,
    )
    
    req, err := http.NewRequest("GET", url, nil)
    if err != nil {
        return nil, err
    }
    
    req.Header.Set("Authorization", "Bearer "+accessToken)
    
    resp, err := c.httpClient.Do(req)
    if err != nil {
        return nil, err
    }
    defer resp.Body.Close()
    
    if resp.StatusCode != http.StatusOK {
        body, _ := io.ReadAll(resp.Body)
        return nil, fmt.Errorf("strava API error: %s", string(body))
    }
    
    var activities []StravaActivity
    if err := json.NewDecoder(resp.Body).Decode(&activities); err != nil {
        return nil, err
    }
    
    return activities, nil
}
```

**`handlers/oauth.go`:**
```go
package handlers

import (
    "fmt"
    "net/http"
    "os"
    "time"
    
    "github.com/labstack/echo/v4"
    "github.com/yourusername/strava-server/db"
    "github.com/yourusername/strava-server/strava"
)

type OAuthHandler struct {
    db            *db.PostgresDB
    stravaClient  *strava.Client
}

func NewOAuthHandler(database *db.PostgresDB) *OAuthHandler {
    return &OAuthHandler{
        db:           database,
        stravaClient: strava.NewClient(),
    }
}

func (h *OAuthHandler) InitiateLogin(c echo.Context) error {
    clientID := os.Getenv("STRAVA_CLIENT_ID")
    redirectURI := os.Getenv("STRAVA_REDIRECT_URI")
    scope := "activity:read_all"
    
    authURL := fmt.Sprintf(
        "https://www.strava.com/oauth/authorize?client_id=%s&response_type=code&redirect_uri=%s&scope=%s",
        clientID, redirectURI, scope,
    )
    
    return c.Redirect(http.StatusTemporaryRedirect, authURL)
}

func (h *OAuthHandler) Callback(c echo.Context) error {
    code := c.QueryParam("code")
    if code == "" {
        return echo.NewHTTPError(http.StatusBadRequest, "No code provided")
    }
    
    // Exchange code for token
    tokenResp, err := h.stravaClient.ExchangeCode(code)
    if err != nil {
        return echo.NewHTTPError(http.StatusInternalServerError, "Failed to exchange code: "+err.Error())
    }
    
    // Save or update user in database
    expiresAt := time.Unix(tokenResp.ExpiresAt, 0)
    
    _, err = h.db.Exec(`
        INSERT INTO users (strava_user_id, access_token, refresh_token, token_expires_at, updated_at)
        VALUES ($1, $2, $3, $4, NOW())
        ON CONFLICT (strava_user_id) 
        DO UPDATE SET 
            access_token = $2,
            refresh_token = $3,
            token_expires_at = $4,
            updated_at = NOW()
    `, tokenResp.Athlete.ID, tokenResp.AccessToken, tokenResp.RefreshToken, expiresAt)
    
    if err != nil {
        return echo.NewHTTPError(http.StatusInternalServerError, "Failed to save user: "+err.Error())
    }
    
    return c.HTML(http.StatusOK, `
        <html>
        <body>
            <h1>Authorization Successful!</h1>
            <p>Your Strava account has been connected.</p>
            <p>User ID: `+fmt.Sprint(tokenResp.Athlete.ID)+`</p>
            <p>You can close this window.</p>
        </body>
        </html>
    `)
}
```

**`handlers/esp32.go`:**
```go
package handlers

import (
    "database/sql"
    "net/http"
    "strconv"
    "time"
    
    "github.com/labstack/echo/v4"
    "github.com/yourusername/strava-server/db"
    "github.com/yourusername/strava-server/models"
)

type ESP32Handler struct {
    db *db.PostgresDB
}

func NewESP32Handler(database *db.PostgresDB) *ESP32Handler {
    return &ESP32Handler{db: database}
}

func (h *ESP32Handler) GetRecentActivities(c echo.Context) error {
    userID, err := strconv.Atoi(c.Param("userId"))
    if err != nil {
        return echo.NewHTTPError(http.StatusBadRequest, "Invalid user ID")
    }
    
    rows, err := h.db.Query(`
        SELECT id, user_id, strava_activity_id, name, type, distance, moving_time, start_date, created_at
        FROM activities
        WHERE user_id = $1
        ORDER BY start_date DESC
        LIMIT 10
    `, userID)
    if err != nil {
        return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
    }
    defer rows.Close()
    
    var activities []models.Activity
    for rows.Next() {
        var a models.Activity
        if err := rows.Scan(&a.ID, &a.UserID, &a.StravaActivityID, &a.Name, &a.Type, 
            &a.Distance, &a.MovingTime, &a.StartDate, &a.CreatedAt); err != nil {
            return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
        }
        activities = append(activities, a)
    }
    
    return c.JSON(http.StatusOK, activities)
}

func (h *ESP32Handler) GetCalendarData(c echo.Context) error {
    userID, err := strconv.Atoi(c.Param("userId"))
    if err != nil {
        return echo.NewHTTPError(http.StatusBadRequest, "Invalid user ID")
    }
    
    year, err := strconv.Atoi(c.Param("year"))
    if err != nil {
        return echo.NewHTTPError(http.StatusBadRequest, "Invalid year")
    }
    
    month, err := strconv.Atoi(c.Param("month"))
    if err != nil {
        return echo.NewHTTPError(http.StatusBadRequest, "Invalid month")
    }
    
    // Get first and last day of month
    firstDay := time.Date(year, time.Month(month), 1, 0, 0, 0, 0, time.UTC)
    lastDay := firstDay.AddDate(0, 1, -1)
    
    rows, err := h.db.Query(`
        SELECT 
            DATE(start_date) as activity_date,
            COUNT(*) as count,
            SUM(distance) as total_distance
        FROM activities
        WHERE user_id = $1
        AND start_date >= $2
        AND start_date <= $3
        GROUP BY DATE(start_date)
        ORDER BY activity_date
    `, userID, firstDay, lastDay)
    if err != nil {
        return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
    }
    defer rows.Close()
    
    var calendarDays []models.CalendarDay
    for rows.Next() {
        var day models.CalendarDay
        var activityDate time.Time
        var count int
        var totalDistance sql.NullFloat64
        
        if err := rows.Scan(&activityDate, &count, &totalDistance); err != nil {
            return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
        }
        
        day.Date = activityDate.Format("2006-01-02")
        day.HasActivity = true
        day.Count = count
        if totalDistance.Valid {
            day.Distance = totalDistance.Float64
        }
        
        calendarDays = append(calendarDays, day)
    }
    
    return c.JSON(http.StatusOK, calendarDays)
}

func (h *ESP32Handler) GetUserStats(c echo.Context) error {
    userID, err := strconv.Atoi(c.Param("userId"))
    if err != nil {
        return echo.NewHTTPError(http.StatusBadRequest, "Invalid user ID")
    }
    
    var stats struct {
        TotalActivities int     `json:"total_activities"`
        TotalDistance   float64 `json:"total_distance"`
        TotalTime       int     `json:"total_time"`
    }
    
    err = h.db.QueryRow(`
        SELECT 
            COUNT(*) as total_activities,
            COALESCE(SUM(distance), 0) as total_distance,
            COALESCE(SUM(moving_time), 0) as total_time
        FROM activities
        WHERE user_id = $1
    `, userID).Scan(&stats.TotalActivities, &stats.TotalDistance, &stats.TotalTime)
    
    if err != nil {
        return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
    }
    
    return c.JSON(http.StatusOK, stats)
}
```

**`handlers/strava.go`:**
```go
package handlers

import (
    "net/http"
    "strconv"
    "time"
    
    "github.com/labstack/echo/v4"
    "github.com/yourusername/strava-server/db"
    "github.com/yourusername/strava-server/models"
    "github.com/yourusername/strava-server/strava"
)

type StravaHandler struct {
    db           *db.PostgresDB
    stravaClient *strava.Client
}

func NewStravaHandler(database *db.PostgresDB) *StravaHandler {
    return &StravaHandler{
        db:           database,
        stravaClient: strava.NewClient(),
    }
}

func (h *StravaHandler) ListUsers(c echo.Context) error {
    rows, err := h.db.Query(`
        SELECT id, strava_user_id, created_at, updated_at
        FROM users
        ORDER BY created_at DESC
    `)
    if err != nil {
        return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
    }
    defer rows.Close()
    
    var users []models.User
    for rows.Next() {
        var u models.User
        if err := rows.Scan(&u.ID, &u.StravaUserID, &u.CreatedAt, &u.UpdatedAt); err != nil {
            return echo.NewHTTPError(http.StatusInternalServerError, err.Error())
        }
        users = append(users, u)
    }
    
    return c.JSON(http.StatusOK, users)
}

func (h *StravaHandler) SyncActivities(c echo.Context) error {
    userID, err := strconv.Atoi(c.Param("userId"))
    if err != nil {
        return echo.NewHTTPError(http.StatusBadRequest, "Invalid user ID")
    }
    
    // Get user's tokens
    var user models.User
    err = h.db.QueryRow(`
        SELECT id, strava_user_id, access_token, refresh_token, token_expires_at
        FROM users WHERE id = $1
    `, userID).Scan(&user.ID, &user.StravaUserID, &user.AccessToken, &user.RefreshToken, &user.TokenExpiresAt)
    
    if err != nil {
        return echo.NewHTTPError(http.StatusNotFound, "User not found")
    }
    
    // Refresh token if expired
    if time.Now().After(user.TokenExpiresAt) {
        tokenResp, err := h.stravaClient.RefreshToken(user.RefreshToken)
        if err != nil {
            return echo.NewHTTPError(http.StatusInternalServerError, "Failed to refresh token: "+err.Error())
        }
        
        user.AccessToken = tokenResp.AccessToken
        user.RefreshToken = tokenResp.RefreshToken
        user.TokenExpiresAt = time.Unix(tokenResp.ExpiresAt, 0)
        
        // Update tokens in database
        _, err = h.db.Exec(`
            UPDATE users 
            SET access_token = $1, refresh_token = $2, token_expires_at = $3, updated_at = NOW()
            WHERE id = $4
        `, user.AccessToken, user.RefreshToken, user.TokenExpiresAt, user.ID)
        
        if err != nil {
            return echo.NewHTTPError(http.StatusInternalServerError, "Failed to update tokens: "+err.Error())
        }
    }
    
    // Fetch activities from Strava (last 30 days)
    thirtyDaysAgo := time.Now().AddDate(0, 0, -30).Unix()
    activities, err := h.stravaClient.GetActivities(user.AccessToken, thirtyDaysAgo, 100)
    if err != nil {
        return echo.NewHTTPError(http.StatusInternalServerError, "Failed to fetch activities: "+err.Error())
    }
    
    // Save activities to database
    saved := 0
    for _, activity := range activities {
        _, err := h.db.Exec(`
            INSERT INTO activities (user_id, strava_activity_id, name, type, distance, moving_time, start_date)
            VALUES ($1, $2, $3, $4, $5, $6, $7)
            ON CONFLICT (strava_activity_id) DO NOTHING
        `, user.ID, activity.ID, activity.Name, activity.Type, activity.Distance, activity.MovingTime, activity.StartDate)
        
        if err == nil {
            saved++
        }
    }
    
    return c.JSON(http.StatusOK, map[string]interface{}{
        "message":        "Sync completed",
        "fetched":        len(activities),
        "saved":          saved,
    })
}
```

### Step 1.7: Build and Run

```bash
# Build
go build -o strava-server

# Run
./strava-server
```

Or run directly:
```bash
go run main.go
```

### Step 1.8: Test OAuth Flow

1. Navigate to `http://localhost:8080/auth/login`
2. Authorize with Strava
3. Get redirected back with success message
4. Note your user ID for ESP32 configuration

### Step 1.9: Sync Activities

```bash
# Using curl (replace USER_ID with your actual ID from database)
curl -u admin:your_admin_password \
  -X POST http://localhost:8080/admin/sync/1
```

### Step 1.10: Test ESP32 API Endpoints

```bash
# Get recent activities
curl -H "X-API-Key: your_secure_api_key_here" \
  http://localhost:8080/api/activities/recent/1

# Get calendar data
curl -H "X-API-Key: your_secure_api_key_here" \
  http://localhost:8080/api/activities/calendar/1/2025/11

# Get stats
curl -H "X-API-Key: your_secure_api_key_here" \
  http://localhost:8080/api/stats/1
```

---

## Part 2: ESP32 MicroPython Setup

### Prerequisites
- ESP32 development board
- USB cable
- Python 3.7+ with esptool

### Step 2.1: Flash MicroPython Firmware

```bash
# Install esptool
pip3 install esptool

# Download firmware
wget https://micropython.org/resources/firmware/esp32-20231005-v1.21.0.bin

# Erase flash
esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash

# Flash MicroPython
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 \
  write_flash -z 0x1000 esp32-20231005-v1.21.0.bin
```

### Step 2.2: ESP32 Project Structure

```
esp32-client/
├── boot.py              # WiFi setup
├── main.py              # Main application
├── api_client.py        # HTTP client for Go server
└── config.py            # Configuration
```

### Step 2.3: ESP32 Code

**`config.py`:**
```python
# config.py
WIFI_SSID = 'YOUR_WIFI_SSID'
WIFI_PASSWORD = 'YOUR_WIFI_PASSWORD'

# Your Go server URL
API_BASE_URL = 'http://your-domain.com'  # or http://your-ip:8080
API_KEY = 'your_secure_api_key_here'

# Your user ID from the database
USER_ID = 1

# Refresh interval (seconds)
REFRESH_INTERVAL = 300  # 5 minutes
```

**`boot.py`:**
```python
# boot.py
import network
import time
from config import WIFI_SSID, WIFI_PASSWORD

def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    
    if not wlan.isconnected():
        print('Connecting to WiFi...')
        wlan.connect(WIFI_SSID, WIFI_PASSWORD)
        
        timeout = 10
        start = time.time()
        while not wlan.isconnected():
            if time.time() - start > timeout:
                print('WiFi connection timeout!')
                return False
            time.sleep(0.5)
    
    print('WiFi connected!')
    print('IP:', wlan.ifconfig()[0])
    return True

# Auto-connect on boot
connect_wifi()
```

**`api_client.py`:**
```python
# api_client.py
import urequests
import json
from config import API_BASE_URL, API_KEY, USER_ID

class StravaAPIClient:
    def __init__(self):
        self.base_url = API_BASE_URL
        self.api_key = API_KEY
        self.user_id = USER_ID
        self.headers = {
            'X-API-Key': self.api_key,
            'Content-Type': 'application/json'
        }
    
    def get_recent_activities(self):
        """Fetch recent activities from server"""
        try:
            url = f'{self.base_url}/api/activities/recent/{self.user_id}'
            response = urequests.get(url, headers=self.headers)
            
            if response.status_code == 200:
                data = response.json()
                response.close()
                return data
            else:
                print(f'Error: {response.status_code}')
                response.close()
                return None
        except Exception as e:
            print('Request failed:', e)
            return None
    
    def get_calendar_data(self, year, month):
        """Fetch calendar data for a specific month"""
        try:
            url = f'{self.base_url}/api/activities/calendar/{self.user_id}/{year}/{month}'
            response = urequests.get(url, headers=self.headers)
            
            if response.status_code == 200:
                data = response.json()
                response.close()
                return data
            else:
                print(f'Error: {response.status_code}')
                response.close()
                return None
        except Exception as e:
            print('Request failed:', e)
            return None
    
    def get_user_stats(self):
        """Fetch user statistics"""
        try:
            url = f'{self.base_url}/api/stats/{self.user_id}'
            response = urequests.get(url, headers=self.headers)
            
            if response.status_code == 200:
                data = response.json()
                response.close()
                return data
            else:
                print(f'Error: {response.status_code}')
                response.close()
                return None
        except Exception as e:
            print('Request failed:', e)
            return None
    
    def health_check(self):
        """Check if server is reachable"""
        try:
            url = f'{self.base_url}/health'
            response = urequests.get(url)
            status = response.status_code == 200
            response.close()
            return status
        except Exception as e:
            print('Health check failed:', e)
            return False
```

**`main.py`:**
```python
# main.py
import time
import gc
from api_client import StravaAPIClient
from config import REFRESH_INTERVAL

def display_activities(activities):
    """Display activities - customize for your LED matrix"""
    if not activities:
        print('No activities found')
        return
    
    print('\n=== Recent Activities ===')
    for activity in activities[:5]:  # Show top 5
        name = activity['name']
        distance_km = activity['distance'] / 1000
        activity_type = activity['type']
        date = activity['start_date'][:10]
        
        print(f'{date} | {activity_type} | {name} | {distance_km:.2f} km')
    print('=' * 40)

def display_calendar(calendar_data, year, month):
    """Display calendar data - customize for your LED matrix"""
    if not calendar_data:
        print(f'No activities in {year}-{month:02d}')
        return
    
    print(f'\n=== Calendar {year}-{month:02d} ===')
    for day in calendar_data:
        date = day['date']
        count = day['count']
        distance_km = day['total_distance'] / 1000
        print(f'{date}: {count} activities, {distance_km:.2f} km')
    print('=' * 40)

def display_stats(stats):
    """Display user stats"""
    if not stats:
        print('No stats available')
        return
    
    total_activities = stats['total_activities']
    total_distance_km = stats['total_distance'] / 1000
    total_hours = stats['total_time'] / 3600
    
    print('\n=== Your Stats ===')
    print(f'Total Activities: {total_activities}')
    print(f'Total Distance: {total_distance_km:.2f} km')
    print(f'Total Time: {total_hours:.2f} hours')
    print('=' * 40)

def main():
    print('Strava Activity Display Starting...')
    
    # Initialize API client
    client = StravaAPIClient()
    
    # Check server health
    if not client.health_check():
        print('Error: Cannot reach server!')
        print('Please check your API_BASE_URL in config.py')
        return
    
    print('Server connection OK!')
    
    while True:
        try:
            print('\nFetching data from server...')
            
            # Get recent activities
            activities = client.get_recent_activities()
            if activities:
                display_activities(activities)
            
            # Get current month calendar
            now = time.localtime()
            year = now[0]
            month = now[1]
            calendar_data = client.get_calendar_data(year, month)
            if calendar_data:
                display_calendar(calendar_data, year, month)
            
            # Get stats
            stats = client.get_user_stats()
            if stats:
                display_stats(stats)
            
            # TODO: Update LED matrix here with the data
            # display_on_matrix(calendar_data)
            
            print(f'\nWaiting {REFRESH_INTERVAL} seconds...')
            gc.collect()  # Free memory
            time.sleep(REFRESH_INTERVAL)
            
        except KeyboardInterrupt:
            print('\nShutting down...')
            break
        except Exception as e:
            print('Error in main loop:', e)
            time.sleep(30)  # Wait before retry

if __name__ == '__main__':
    main()
```

### Step 2.4: Upload to ESP32

```bash
# Install ampy or mpremote
pip3 install adafruit-ampy

# Upload files
ampy --port /dev/ttyUSB0 put config.py
ampy --port /dev/ttyUSB0 put boot.py
ampy --port /dev/ttyUSB0 put api_client.py
ampy --port /dev/ttyUSB0 put main.py

# Or use mpremote
pip3 install mpremote
mpremote connect /dev/ttyUSB0 fs cp config.py :config.py
mpremote connect /dev/ttyUSB0 fs cp boot.py :boot.py
mpremote connect /dev/ttyUSB0 fs cp api_client.py :api_client.py
mpremote connect /dev/ttyUSB0 fs cp main.py :main.py
```

### Step 2.5: Test ESP32

Connect to REPL:
```bash
screen /dev/ttyUSB0 115200
```

Press reset button or run:
```python
>>> import main
```

You should see:
```
WiFi connected!
IP: 192.168.1.100
Strava Activity Display Starting...
Server connection OK!
Fetching data from server...
=== Recent Activities ===
...
```

---

## Part 3: Deployment

### Option A: Home Lab

**Using systemd service:**

Create `/etc/systemd/system/strava-server.service`:
```ini
[Unit]
Description=Strava API Server
After=network.target postgresql.service

[Service]
Type=simple
User=your-user
WorkingDirectory=/home/your-user/strava-server
ExecStart=/home/your-user/strava-server/strava-server
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl daemon-reload
sudo systemctl enable strava-server
sudo systemctl start strava-server
sudo systemctl status strava-server
```

**Setup reverse proxy with Nginx:**

```nginx
# /etc/nginx/sites-available/strava-server
server {
    listen 80;
    server_name yourdomain.com;

    location / {
        proxy_pass http://localhost:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

Enable site:
```bash
sudo ln -s /etc/nginx/sites-available/strava-server /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

**Setup SSL with Let's Encrypt:**
```bash
sudo apt install certbot python3-certbot-nginx
sudo certbot --nginx -d yourdomain.com
```

### Option B: Cloud Deployment (Fly.io)

Create `fly.toml`:
```toml
app = "your-strava-server"
primary_region = "sjc"

[build]
  builder = "paketobuildpacks/builder:base"

[env]
  PORT = "8080"

[http_service]
  internal_port = 8080
  force_https = true
  auto_stop_machines = true
  auto_start_machines = true
  min_machines_running = 0

[[services]]
  protocol = "tcp"
  internal_port = 8080

  [[services.ports]]
    port = 80
    handlers = ["http"]

  [[services.ports]]
    port = 443
    handlers = ["tls", "http"]
```

Deploy:
```bash
# Install flyctl
curl -L https://fly.io/install.sh | sh

# Login
flyctl auth login

# Create app
flyctl launch

# Set secrets
flyctl secrets set STRAVA_CLIENT_ID=your_id
flyctl secrets set STRAVA_CLIENT_SECRET=your_secret
flyctl secrets set ESP32_API_KEY=your_key
flyctl secrets set DB_HOST=your_db_host
flyctl secrets set DB_PASSWORD=your_db_pass

# Deploy
flyctl deploy
```

### Option C: Railway.app

1. Push code to GitHub
2. Connect Railway to your repo
3. Add PostgreSQL database
4. Set environment variables
5. Deploy automatically on push

### Option D: Docker

Create `Dockerfile`:
```dockerfile
FROM golang:1.21-alpine AS builder

WORKDIR /app
COPY go.* ./
RUN go mod download
COPY . .
RUN go build -o strava-server

FROM alpine:latest
RUN apk --no-cache add ca-certificates
WORKDIR /root/
COPY --from=builder /app/strava-server .
EXPOSE 8080
CMD ["./strava-server"]
```

Create `docker-compose.yml`:
```yaml
version: '3.8'

services:
  app:
    build: .
    ports:
      - "8080:8080"
    environment:
      - PORT=8080
      - DB_HOST=db
      - DB_PORT=5432
      - DB_USER=strava
      - DB_PASSWORD=your_password
      - DB_NAME=strava_db
    env_file:
      - .env
    depends_on:
      - db

  db:
    image: postgres:15-alpine
    environment:
      - POSTGRES_USER=strava
      - POSTGRES_PASSWORD=your_password
      - POSTGRES_DB=strava_db
    volumes:
      - postgres_data:/var/lib/postgresql/data
      - ./schema.sql:/docker-entrypoint-initdb.d/schema.sql

volumes:
  postgres_data:
```

Deploy:
```bash
docker-compose up -d
```

---

## Part 4: LED Matrix Integration (Future)

When you're ready to add LED matrix display:

### MAX7219 LED Matrix Example

**Install driver:**
```python
# Upload max7219.py library to ESP32
# Available at: https://github.com/mcauser/micropython-max7219
```

**Update `main.py`:**
```python
from machine import Pin, SPI
import max7219

# Initialize LED matrix
spi = SPI(1, baudrate=10000000, polarity=0, phase=0)
display = max7219.Matrix8x8(spi, Pin(15), 4)  # 4 matrices

def display_on_matrix(calendar_data):
    """Display calendar on LED matrix"""
    display.fill(0)
    
    for i, day in enumerate(calendar_data[:32]):  # 32 LEDs
        if day['has_activity']:
            x = i % 8
            y = i // 8
            display.pixel(x, y, 1)
    
    display.show()
```

---

## Testing End-to-End

1. **Start Go server**
   ```bash
   ./strava-server
   ```

2. **Complete OAuth flow**
   - Visit http://your-domain.com/auth/login
   - Authorize with Strava

3. **Sync activities**
   ```bash
   curl -u admin:password -X POST http://your-domain.com/admin/sync/1
   ```

4. **Test API from ESP32**
   - Connect ESP32
   - Watch REPL output
   - Should fetch and display activities

---

## Troubleshooting

### Go Server Issues

**Port already in use:**
```bash
lsof -i :8080
kill -9 <PID>
```

**Database connection failed:**
```bash
psql -U your_user -d strava_db -h localhost
# Verify credentials
```

**OAuth redirect mismatch:**
- Check Strava app settings
- Update STRAVA_REDIRECT_URI in .env
- Must match exactly (including http/https)

### ESP32 Issues

**WiFi not connecting:**
```python
>>> import network
>>> wlan = network.WLAN(network.STA_IF)
>>> wlan.active(True)
>>> wlan.scan()
```

**urequests not found:**
```bash
mpremote connect /dev/ttyUSB0 mip install urequests
```

**Memory errors:**
```python
>>> import gc
>>> gc.collect()
>>> gc.mem_free()
```

**API key authentication failed:**
- Verify API_KEY matches in both .env and config.py
- Check request headers in Go server logs

---

## Security Considerations

1. **Use HTTPS in production** - Let's Encrypt is free
2. **Generate strong API key** - `openssl rand -hex 32`
3. **Use environment variables** - Never commit secrets
4. **Rate limiting** - Add middleware to prevent abuse
5. **Database backups** - Regular automated backups
6. **Token encryption** - Consider encrypting tokens in database
7. **Firewall rules** - Restrict database access

---

## Next Steps

1. Add automated activity sync (cron job or background worker)
2. Implement webhook handler for real-time Strava updates
3. Add multiple user support
4. Create web dashboard for configuration
5. Implement LED matrix display patterns
6. Add activity type filtering
7. Create mobile app companion

---

## Resources

- [Go Echo Framework](https://echo.labstack.com/)
- [Strava API Documentation](https://developers.strava.com/docs/reference/)
- [MicroPython ESP32 Guide](https://docs.micropython.org/en/latest/esp32/quickref.html)
- [PostgreSQL Documentation](https://www.postgresql.org/docs/)

Good luck with your project!
