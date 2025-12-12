#include <cstdio>
#include <chrono>
#include <thread>
#include <vector>
#include <memory>
#include <csignal>
#include <string>
#include <cstring>

// Terminal input handling (raw mode for j/k keys)
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#include "TerminalDisplay.h"
#include "Panel.h"
#include "CalendarPanel.h"
#include "Transition.h"
#include "MockData.h"
#include "StravaClient.h"

// Configuration - can be overridden with environment variables
static const char* DEFAULT_SERVER_URL = "http://192.168.68.100:8080";
static const char* DEFAULT_API_KEY = "9f267ca3adb01e394f917902588fc920ae3669e1889f360f16bc1792768779e6";
static const int DEFAULT_USER_ID = 1;

// Global for signal handling
static volatile bool g_running = true;
static struct termios g_originalTermios;

void signalHandler(int) {
    g_running = false;
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &g_originalTermios);
    struct termios raw = g_originalTermios;
    raw.c_lflag &= ~(ECHO | ICANON);  // Disable echo and canonical mode
    raw.c_cc[VMIN] = 0;   // Non-blocking
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    
    // Set non-blocking
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_originalTermios);
}

// Check if a key is available and read it
char readKey() {
    char c = 0;
    if (read(STDIN_FILENO, &c, 1) == 1) {
        return c;
    }
    return 0;
}

// Cool stats panel with visual elements
class StatsPanel : public Panel {
public:
    StatsPanel() : animTime_(0) {
        // Mock data for now - weekly miles per day (Sun-Sat)
        weeklyMiles_[0] = 0;    // Sun
        weeklyMiles_[1] = 3.2f; // Mon
        weeklyMiles_[2] = 0;    // Tue
        weeklyMiles_[3] = 5.1f; // Wed
        weeklyMiles_[4] = 0;    // Thu
        weeklyMiles_[5] = 4.5f; // Fri
        weeklyMiles_[6] = 6.2f; // Sat
        
        totalMiles_ = 19.0f;
        goalMiles_ = 25.0f;
        totalRuns_ = 4;
    }
    
    void render(IDisplay* display) override {
        display->fillScreen(0);
        
        uint16_t white = display->color565(255, 255, 255);
        uint16_t green = display->color565(0, 255, 0);
        uint16_t dimGreen = display->color565(0, 100, 0);
        uint16_t orange = display->color565(255, 150, 0);
        uint16_t gray = display->color565(80, 80, 80);
        uint16_t cyan = display->color565(0, 200, 255);
        
        // === LEFT SIDE: Weekly bar chart ===
        // Days: S M T W T F S (7 bars)
        // Each bar max height = 20px, width = 3px, gap = 1px
        // Total width = 7*3 + 6*1 = 27px
        int barStartX = 2;
        int barBaseY = 26;  // Bottom of bars
        int barWidth = 3;
        int barGap = 1;
        int maxBarHeight = 18;
        
        // Find max miles for scaling
        float maxMiles = 1.0f;
        for (int i = 0; i < 7; i++) {
            if (weeklyMiles_[i] > maxMiles) maxMiles = weeklyMiles_[i];
        }
        
        // Draw bars
        for (int i = 0; i < 7; i++) {
            int x = barStartX + i * (barWidth + barGap);
            int height = (int)((weeklyMiles_[i] / maxMiles) * maxBarHeight);
            if (height < 1 && weeklyMiles_[i] > 0) height = 1;
            
            if (height > 0) {
                // Filled bar with gradient effect
                uint16_t barColor = (i == 6) ? orange : green;  // Today (Sat) is orange
                display->fillRect(x, barBaseY - height, barWidth, height, barColor);
            }
            
            // Draw base tick mark
            display->drawPixel(x + 1, barBaseY + 1, gray);
        }
        
        // Weekend markers at bottom
        display->setTextSize(1);
        display->setTextColor(gray);
        for (int i = 0; i < 7; i++) {
            int x = barStartX + i * (barWidth + barGap);
            // Draw single pixel "dots" for weekend days
            if (i == 0 || i == 6) {
                display->drawPixel(x + 1, barBaseY + 3, gray);
            }
        }
        
        // === RIGHT SIDE: Stats summary ===
        int rightX = 34;
        
        // Total miles - big number
        display->setTextColor(white);
        display->setCursor(rightX, 2);
        char milesStr[8];
        snprintf(milesStr, sizeof(milesStr), "%d", (int)totalMiles_);
        display->print(milesStr);
        
        // "mi" label
        display->setTextColor(gray);
        int milesWidth = strlen(milesStr) * 6;
        display->setCursor(rightX + milesWidth + 1, 2);
        display->print("mi");
        
        // Progress bar toward goal
        int progY = 11;
        int progWidth = 26;
        int progHeight = 3;
        float progress = totalMiles_ / goalMiles_;
        if (progress > 1.0f) progress = 1.0f;
        int filledWidth = (int)(progress * progWidth);
        
        // Background
        display->fillRect(rightX, progY, progWidth, progHeight, dimGreen);
        // Filled portion
        if (filledWidth > 0) {
            display->fillRect(rightX, progY, filledWidth, progHeight, green);
        }
        
        // Goal indicator at end
        display->drawPixel(rightX + progWidth - 1, progY - 1, white);
        display->drawPixel(rightX + progWidth - 1, progY + progHeight, white);
        
        // Runs count with icon
        display->setTextColor(cyan);
        display->setCursor(rightX, 17);
        char runsStr[8];
        snprintf(runsStr, sizeof(runsStr), "%dr", totalRuns_);
        display->print(runsStr);
        
        // Animated "pulse" dot to show it's live
        int pulsePhase = (animTime_ / 500) % 2;
        if (pulsePhase == 0) {
            display->drawPixel(61, 2, green);
        }
    }
    
    void update(uint32_t deltaMs) override {
        animTime_ += deltaMs;
    }
    
    const char* name() const override { return "Stats"; }
    
    void setWeeklyData(float miles[7], float total, float goal, int runs) {
        for (int i = 0; i < 7; i++) {
            weeklyMiles_[i] = miles[i];
        }
        totalMiles_ = total;
        goalMiles_ = goal;
        totalRuns_ = runs;
    }
    
private:
    uint32_t animTime_;
    float weeklyMiles_[7];
    float totalMiles_;
    float goalMiles_;
    int totalRuns_;
};

// Welcome/splash panel
class SplashPanel : public Panel {
public:
    SplashPanel() : animTime_(0) {}
    
    void render(IDisplay* display) override {
        display->fillScreen(0);
        
        // Animated color based on time
        int phase = (animTime_ / 50) % 6;
        uint16_t colors[] = {
            display->color565(255, 100, 100),
            display->color565(255, 255, 100),
            display->color565(100, 255, 100),
            display->color565(100, 255, 255),
            display->color565(100, 100, 255),
            display->color565(255, 100, 255)
        };
        
        uint16_t titleColor = colors[phase];
        
        // Title
        display->setCursor(8, 8);
        display->setTextColor(titleColor);
        display->setTextSize(1);
        display->print("MATRIX");
        
        display->setCursor(12, 16);
        display->print("MILES");
        
        // Instructions
        display->setCursor(4, 26);
        display->setTextColor(display->color565(128, 128, 128));
        display->print("J K Q");
    }
    
    void update(uint32_t deltaMs) override {
        animTime_ += deltaMs;
    }
    
    const char* name() const override { return "Splash"; }
    
private:
    uint32_t animTime_;
};

void printHelp() {
    printf("\n");
    printf("Matrix Simulator Controls:\n");
    printf("  j     - Next panel (down)\n");
    printf("  k     - Previous panel (up)\n");
    printf("  r     - Refresh data from server\n");
    printf("  q     - Quit\n");
    printf("  1-3   - Jump to panel\n");
    printf("\n");
}

std::string getEnvOrDefault(const char* name, const char* defaultValue) {
    const char* value = getenv(name);
    return value ? std::string(value) : std::string(defaultValue);
}

int main() {
    // Setup signal handler for clean exit
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Get configuration from environment or defaults
    std::string serverUrl = getEnvOrDefault("STRAVA_SERVER_URL", DEFAULT_SERVER_URL);
    std::string apiKey = getEnvOrDefault("STRAVA_API_KEY", DEFAULT_API_KEY);
    int userId = DEFAULT_USER_ID;
    const char* userIdEnv = getenv("STRAVA_USER_ID");
    if (userIdEnv) {
        userId = std::stoi(userIdEnv);
    }
    
    printf("Connecting to server: %s\n", serverUrl.c_str());
    printf("User ID: %d\n", userId);
    
    // Create Strava client
    StravaClient stravaClient(serverUrl, apiKey, userId);
    
    // Test connection and sync time
    if (stravaClient.testConnection()) {
        printf("Server connection: OK\n");
        if (stravaClient.syncTime()) {
            int day, month, year;
            stravaClient.getSyncedDate(day, month, year);
            printf("Server time: %04d-%02d-%02d\n", year, month, day);
        }
    } else {
        printf("Server connection: FAILED (%s)\n", stravaClient.getLastError().c_str());
        printf("Using mock data instead.\n");
    }

    // Get latest Month's data
    stravaClient.syncActivities();
    
    // Create display
    TerminalDisplay display;
    
    // Create panels
    std::vector<std::unique_ptr<Panel>> panels;
    
    // Panel 0: Splash
    panels.push_back(std::make_unique<SplashPanel>());
    
    // Panel 1: Calendar with real data
    auto calendar = std::make_unique<CalendarPanel>();
    calendar->setStravaClient(&stravaClient);
    
    // Fetch initial data
    auto activities = stravaClient.fetchCalendarData(calendar->getYear(), calendar->getMonth());
    if (activities.empty()) {
        printf("No activities from server, using mock data.\n");
        calendar->setActivities(generateMockActivities());
    } else {
        printf("Loaded %zu activity days from server.\n", activities.size());
        calendar->setActivities(activities);
    }
    panels.push_back(std::move(calendar));
    
    // Panel 2: Stats
    panels.push_back(std::make_unique<StatsPanel>());
    
    int currentPanel = 0;
    Transition transition;
    
    // Setup terminal
    enableRawMode();
    display.initTerminal();
    
    // Print initial help (will be overwritten)
    printHelp();
    
    // Timing
    auto lastTime = std::chrono::steady_clock::now();
    const auto frameTime = std::chrono::milliseconds(16);  // ~60 FPS
    
    // Initial render
    panels[currentPanel]->onEnter();
    
    while (g_running) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime);
        lastTime = now;
        uint32_t deltaMs = static_cast<uint32_t>(elapsed.count());
        
        // Handle input
        char key = readKey();
        if (key != 0) {
            int targetPanel = currentPanel;
            TransitionType transType = TransitionType::None;
            
            switch (key) {
                case 'q':
                case 'Q':
                case 27:  // ESC
                    g_running = false;
                    break;
                    
                case 'j':
                case 'J':
                    // Next panel (down)
                    if (!transition.isActive()) {
                        targetPanel = (currentPanel + 1) % panels.size();
                        transType = TransitionType::SlideUp;
                    }
                    break;
                    
                case 'k':
                case 'K':
                    // Previous panel (up)
                    if (!transition.isActive()) {
                        targetPanel = (currentPanel - 1 + panels.size()) % panels.size();
                        transType = TransitionType::SlideDown;
                    }
                    break;
                    
                case '1':
                    if (!transition.isActive() && currentPanel != 0) {
                        targetPanel = 0;
                        transType = TransitionType::CrossFade;
                    }
                    break;
                    
                case '2':
                    if (!transition.isActive() && currentPanel != 1) {
                        targetPanel = 1;
                        transType = TransitionType::CrossFade;
                    }
                    break;
                    
                case '3':
                    if (!transition.isActive() && currentPanel != 2) {
                        targetPanel = 2;
                        transType = TransitionType::CrossFade;
                    }
                    break;
                    
                case 'r':
                case 'R':
                    // Refresh calendar data from server
                    if (currentPanel == 1) {
                        // Get the CalendarPanel and refresh
                        CalendarPanel* cal = dynamic_cast<CalendarPanel*>(panels[1].get());
                        if (cal) {
                            cal->refreshFromServer();
                        }
                    }
                    break;
            }
            
            // Start transition if panel changed
            if (targetPanel != currentPanel && transType != TransitionType::None) {
                panels[currentPanel]->onExit();
                panels[targetPanel]->onEnter();
                transition.start(transType, panels[currentPanel].get(),
                               panels[targetPanel].get(), &display, 300);
                currentPanel = targetPanel;
            }
        }
        
        // Update
        if (transition.isActive()) {
            transition.update(deltaMs);
        } else {
            panels[currentPanel]->update(deltaMs);
        }
        
        // Render
        if (transition.isActive()) {
            transition.render(&display);
        } else {
            panels[currentPanel]->render(&display);
        }
        
        display.show();
        
        // Print status below display
        printf("Panel: %s (%d/%zu)  |  j:next  k:prev  q:quit\033[K\n",
               panels[currentPanel]->name(), currentPanel + 1, panels.size());
        
        // Frame rate limiting
        auto frameEnd = std::chrono::steady_clock::now();
        auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - now);
        if (frameDuration < frameTime) {
            std::this_thread::sleep_for(frameTime - frameDuration);
        }
    }
    
    // Cleanup
    display.restoreTerminal();
    disableRawMode();
    
    printf("\nSimulator exited.\n");
    return 0;
}
