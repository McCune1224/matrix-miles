# Setup Guide for Strava API HTTP Client in C

This guide outlines the steps to set up and build a simple HTTP client in C to consume the Strava API using your personal credentials from the `.env` file.

## Prerequisites
- Linux environment (Ubuntu/Debian recommended)
- GCC compiler
- Internet connection for API calls


## Step 1: Create the C Program
Create a file named `strava_client.c` with the following code structure:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

// Function to read .env file and get value by key
char* get_env_value(const char* key) {
    // Implementation to parse .env file
}

// Function to refresh access token
char* refresh_token(const char* client_id, const char* client_secret, const char* refresh_token) {
    // Use libcurl to POST to Strava token endpoint
    // Parse JSON response for access_token
}

// Function to make API calls
void get_activities(const char* access_token) {
    // Use libcurl to GET /athlete/activities
    // Parse and print JSON response
}

int main() {
    // Load env vars
    char* client_id = get_env_value("CLIENT_ID");
    char* client_secret = get_env_value("CLIENT_SECRET");
    char* refresh_token_val = get_env_value("REFRESH_TOKEN");

    // Get access token
    char* access_token = refresh_token(client_id, client_secret, refresh_token_val);

    // Fetch activities
    get_activities(access_token);

    // Cleanup
    free(client_id);
    free(client_secret);
    free(refresh_token_val);
    free(access_token);

    return 0;
}
```

## Step 4: Implement Helper Functions
Fill in the implementations for `get_env_value`, `refresh_token`, and `get_activities` based on libcurl and cJSON documentation.

## Step 5: Compile the Program
Compile with:

```bash
gcc -o strava_client strava_client.c -lcurl -lcjson
```

## Step 6: Run the Client
Execute the program:

```bash
./strava_client
```

This will refresh the access token and fetch your recent activities from Strava.

## Notes
- Handle errors appropriately in the code (e.g., check curl return codes, JSON parsing errors).
- Strava API rate limits apply; avoid excessive calls.
- For full implementation details, refer to Strava API docs: https://developers.strava.com/docs/reference/
