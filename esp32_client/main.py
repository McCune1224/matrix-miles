# main.py - Main application logic

import time
import gc
from esp32_client.api_client import StravaClient
from esp32_client.config import REFRESH_INTERVAL_SECONDS


def format_distance(meters: float) -> str:
    """Convert meters to km with 2 decimal places"""
    return f"{meters / 1000:.2f}"


def format_duration(seconds: int) -> str:
    """Convert seconds to hours:minutes"""
    hours = seconds // 3600
    minutes = (seconds % 3600) // 60
    return f"{hours}h {minutes}m"


def display_activities(activities):
    """Display recent activities in console"""
    if not activities:
        print("No activities found")
        return

    print("\n" + "=" * 50)
    print("RECENT ACTIVITIES")
    print("=" * 50)

    for i, activity in enumerate(activities, 1):
        name = activity.get("name", "Unknown")
        activity_type = activity.get("type", "Unknown")
        distance_km = format_distance(activity.get("distance", 0))
        duration = format_duration(activity.get("moving_time", 0))
        date = activity.get("start_date", "")[:10]

        print(f"{i}. {name}")
        print(f"   Type: {activity_type} | Date: {date}")
        print(f"   Distance: {distance_km} km | Duration: {duration}")
        print()


def display_stats(stats):
    """Display user statistics"""
    if not stats:
        print("No stats available")
        return

    total_activities = stats.get("total_activities", 0)
    total_distance = format_distance(stats.get("total_distance", 0))
    total_time = format_duration(stats.get("total_time", 0))

    print("\n" + "=" * 50)
    print("YOUR STATS")
    print("=" * 50)
    print(f"Total Activities: {total_activities}")
    print(f"Total Distance: {total_distance} km")
    print(f"Total Time: {total_time}")
    print("=" * 50)


def main():
    """Main application loop"""
    print("\n" + "=" * 50)
    print("ESP32 Strava Activity Display")
    print("=" * 50)

    # Initialize client
    client = StravaClient()

    # Test server connection
    print("\nTesting server connection...")
    if not client.health_check():
        print("\nERROR: Cannot reach server!")
        print("Please check:")
        print("1. API_BASE_URL in config.py is correct")
        print("2. Go server is running")
        print("3. ESP32 can reach the server (firewall/network)")
        return

    print("✓ Server connection successful!")

    # Main loop
    iteration = 0
    while True:
        try:
            iteration += 1
            print(f"\n{'=' * 50}")
            print(f"Update #{iteration} - {time.localtime()}")
            print("=" * 50)

            # Fetch and display recent activities
            print("\nFetching recent activities...")
            activities = client.get_recent_activities(limit=5)
            if activities:
                display_activities(activities)
            else:
                print("Failed to fetch activities")

            # Fetch and display stats
            print("\nFetching stats...")
            stats = client.get_stats()
            if stats:
                display_stats(stats)
            else:
                print("Failed to fetch stats")

            # TODO: Add LED matrix display here
            # display_on_matrix(activities)

            # Free memory
            free_mem = gc.collect()
            # free_mem = gc.mem_free()
            print(f"\nFree memory: {free_mem} bytes")
            print(f"Next update in {REFRESH_INTERVAL_SECONDS} seconds...")

            time.sleep(REFRESH_INTERVAL_SECONDS)

        except KeyboardInterrupt:
            print("\n\nShutting down...")
            print("Press Ctrl+C again to exit to REPL")
            break

        except Exception as e:
            print(f"\nERROR in main loop: {e}")
            print("Waiting 30 seconds before retry...")
            time.sleep(30)


# Run main if executed directly
if __name__ == "__main__":
    main()
