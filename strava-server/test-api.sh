#!/bin/bash
# API Testing Script for Strava Server
# Usage: ./test-api.sh [BASE_URL]

set -e

# Configuration
BASE_URL="${1:-http://localhost:8080}"
API_KEY=${ESP32_API_KEY}
ADMIN_USER="${ADMIN_USERNAME:-admin}"
ADMIN_PASS="${ADMIN_PASSWORD:-changeme}"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Helper function to print test results
test_endpoint() {
    local name="$1"
    local method="$2"
    local endpoint="$3"
    shift 3
    local args=("$@")
    
    echo -e "${YELLOW}Testing: ${name}${NC}"
    echo "Request: ${method} ${endpoint}"
    
    response=$(curl -s -w "\n%{http_code}" "${args[@]}" -X "${method}" "${BASE_URL}${endpoint}")
    http_code=$(echo "$response" | tail -n1)
    body=$(echo "$response" | head -n-1)
    
    if [[ $http_code -ge 200 && $http_code -lt 300 ]]; then
        echo -e "${GREEN}✓ Status: ${http_code}${NC}"
        echo "Response: ${body}" | jq '.' 2>/dev/null || echo "${body}"
    else
        echo -e "${RED}✗ Status: ${http_code}${NC}"
        echo "Response: ${body}"
    fi
    echo ""
}

echo "========================================"
echo "Strava Server API Tests"
echo "Base URL: ${BASE_URL}"
echo "========================================"
echo ""

# 1. Health Check (Public)
test_endpoint "Health Check" "GET" "/health"

# 2. OAuth Login (Public)
echo -e "${YELLOW}OAuth Login URL (open in browser):${NC}"
echo "${BASE_URL}/auth/login"
echo ""

# 3. Get Recent Activities (Protected - requires API key)
echo -e "${YELLOW}Note: Replace 'USER_ID' with actual user ID from your database${NC}"
test_endpoint "Get Recent Activities (requires API key)" "GET" "/api/activities/recent/1" \
    -H "X-API-Key: ${API_KEY}"

# 4. Get Calendar Data (Protected - requires API key)
test_endpoint "Get Calendar Data (requires API key)" "GET" "/api/activities/calendar/1/2024/11" \
    -H "X-API-Key: ${API_KEY}"

# 5. Get User Stats (Protected - requires API key)
test_endpoint "Get User Stats (requires API key)" "GET" "/api/stats/1" \
    -H "X-API-Key: ${API_KEY}"

# 6. Sync Activities (Admin - requires basic auth)
test_endpoint "Sync Activities (admin)" "POST" "/admin/sync/1" \
    -u "${ADMIN_USER}:${ADMIN_PASS}"

# 7. View Recent Logs (Admin)
test_endpoint "Get Recent Logs (admin)" "GET" "/admin/logs" \
    -u "${ADMIN_USER}:${ADMIN_PASS}"

# 8. View Logs by Level (Admin)
test_endpoint "Get Error Logs (admin)" "GET" "/admin/logs/level/error" \
    -u "${ADMIN_USER}:${ADMIN_PASS}"

# 9. View Logs by User (Admin)
test_endpoint "Get User Logs (admin)" "GET" "/admin/logs/user/1" \
    -u "${ADMIN_USER}:${ADMIN_PASS}"

echo "========================================"
echo "Test Summary"
echo "========================================"
echo "Public endpoints:"
echo "  - GET  /health"
echo "  - GET  /auth/login"
echo "  - GET  /auth/callback"
echo ""
echo "API endpoints (X-API-Key header required):"
echo "  - GET  /api/activities/recent/:userId"
echo "  - GET  /api/activities/calendar/:userId/:year/:month"
echo "  - GET  /api/stats/:userId"
echo ""
echo "Admin endpoints (Basic Auth required):"
echo "  - POST /admin/sync/:userId"
echo "  - GET  /admin/logs"
echo "  - GET  /admin/logs/level/:level"
echo "  - GET  /admin/logs/user/:userId"
echo "========================================"
