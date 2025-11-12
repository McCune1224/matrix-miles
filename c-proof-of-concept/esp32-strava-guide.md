# ESP-32 Strava Calendar LED Matrix Guide

## Key C Concepts to Learn for ESP-32 Projects
Focus on embedded C fundamentals first, then ESP-IDF specifics. Prioritize hands-on practice with small projects.

1. **Core C Syntax**: Variables, data types, loops, conditionals, functions, pointers, arrays, structs, and memory allocation (malloc/free). Understand stack vs. heap.
2. **Embedded C Essentials**: Volatile variables, bit manipulation, interrupts, GPIO control, and low-power modes. Learn about registers and direct hardware access.
3. **Concurrency**: Threads/tasks (via FreeRTOS in ESP-IDF), semaphores, and mutexes for multitasking.
4. **Networking**: WiFi setup, TCP/IP sockets, and HTTP client libraries for API calls.
5. **Time and Scheduling**: RTC (real-time clock), timers, and date/time handling for calendar logic.
6. **Data Handling**: String manipulation, JSON parsing (use cJSON or similar), and basic data structures for API responses.

## ESP-IDF Framework (Espressif IoT Development Framework)
- Install ESP-IDF (via Git) and set up the toolchain (GCC for Xtensa).
- Key components: WiFi, HTTP client, NTP for time sync, GPIO drivers for LED matrix.
- Build system: Use `idf.py` for compiling/flashing. Learn CMake basics for project structure.
- For LED matrix: Use SPI/I2C drivers; libraries like Adafruit_GFX or custom drivers for displays (e.g., MAX7219 for dot matrix).
- API Integration: Use `esp_http_client` for Strava REST API (OAuth2 for auth, GET requests for activities).

## Project-Specific Learning Path
1. **Setup Environment**: Install ESP-IDF, configure Neovim with LSP (clangd) for C autocompletion/debugging.
2. **Basic LED Control**: Blink LEDs, then drive a matrix (e.g., 8x8 grid) using GPIO/SPI.
3. **WiFi & HTTP**: Connect to WiFi, make simple HTTP requests (test with a public API).
4. **Strava API**: Register an app, handle OAuth, fetch activity data (filter runs by date).
5. **Calendar Logic**: Use time libraries to map runs to grid positions; store data in flash/NVS.
6. **Integration**: Combine networking, display, and time handling; handle errors/reconnects.

## Neovim Setup for C Development
- Install plugins: `nvim-lspconfig` for clangd, `nvim-cmp` for completion, `nvim-dap` for debugging.
- Configure LSP: Point clangd to ESP-IDF includes (add to compile_commands.json).
- Use `make` or `idf.py` integration for builds; set up keybindings for compilation/flashing.

Start with ESP-IDF docs (esp-idf.readthedocs.io) and examples. Build incrementally: LED blink → WiFi connect → API fetch → display grid. Practice debugging with GDB.

## Consuming Strava API with C (Standalone)
While waiting for hardware, practice API integration on your desktop using standard C libraries.

### Prerequisites
- Install libcurl (for HTTP requests): `sudo apt install libcurl4-openssl-dev` (Ubuntu/Debian).
- Install cJSON (for JSON parsing): Clone from GitHub, build and install.
- Register a Strava app at developers.strava.com to get client ID/secret.

### Steps
1. **OAuth2 Authentication**: Use libcurl to POST to Strava's token endpoint with code (from manual auth flow) to get access token. Store token securely.
2. **Fetch Activities**: Use libcurl to GET `/api/v3/athlete/activities` with Bearer token. Include query params like `after` (Unix timestamp) for date filtering.
3. **Parse Response**: Use cJSON to parse JSON array of activities. Extract dates and activity types (filter for runs).
4. **Calendar Mapping**: Use `<time.h>` to handle dates; map run days to a grid (e.g., 7x5 for weeks/months).
5. **Example Code Structure**:
   - Include headers: `<curl/curl.h>`, `<cjson/cJSON.h>`, `<time.h>`.
   - Initialize curl, set headers (Authorization), perform request.
   - Parse JSON, iterate activities, check `type == "Run"` and extract `start_date`.
   - Print or store run dates for calendar logic.

Compile with: `gcc -o strava_fetch strava_fetch.c -lcurl -lcjson`. Test with your Strava data. This builds API skills without hardware.