"""
Calendar Display Module

Renders activity data as a calendar view on the MatrixPortal M4 LED matrix.
Handles layout, color coding, and text rendering.
"""

import board
import displayio
import time
from adafruit_display_text import label
from adafruit_bitmap_font import bitmap_font


class CalendarDisplay:
    """Manages LED matrix display for calendar view"""

    def __init__(self, width=64, height=32):
        """
        Initialize the display

        Args:
            width (int): Display width in pixels
            height (int): Display height in pixels
        """
        self.width = width
        self.height = height

        # Get the display
        self.display = board.DISPLAY
        self.display.brightness = 1.0

        # Create main group
        self.group = displayio.Group()
        self.display.show(self.group)

        print(f"[Display] Initialized {width}x{height}")

        # Try to load font (falls back to built-in if not available)
        try:
            self.font = bitmap_font.load_font("/fonts/font5x8.bdf")
        except Exception:
            print("[Display] Using built-in font (bitmap font not found)")
            self.font = None

    def clear(self):
        """Clear the display"""
        self.group.pop()

    def show_message(self, message, duration=0):
        """
        Show a simple text message on the display

        Args:
            message (str): Message to display
            duration (int): Duration to show in seconds (0 = permanent until next update)
        """
        self.group.pop()

        # Create text label
        text_area = label.Label(
            self.font or bitmap_font.load_font("/fonts/font5x8.bdf"),
            text=message,
            color=0xFFFFFF,
        )

        # Center the text
        text_area.x = (self.width - len(message) * 5) // 2
        text_area.y = self.height // 2

        self.group.append(text_area)

        if duration > 0:
            time.sleep(duration)

    def render_calendar(self, activities):
        """
        Render activity data as a calendar view

        Args:
            activities (list): List of activity dictionaries with at least:
                - "id": unique identifier
                - "name": activity name
                - "activity_date": date in YYYY-MM-DD format
                - "type": activity type (run, bike, swim, etc.)
        """
        self.group.pop()

        print(f"[Display] Rendering {len(activities)} activities")

        # Extract activity dates
        activity_dates = set()
        activity_map = {}

        for activity in activities:
            try:
                date_str = activity.get("activity_date", "")
                activity_id = activity.get("id", 0)

                if date_str:
                    # Parse date (YYYY-MM-DD)
                    parts = date_str.split("-")
                    if len(parts) == 3:
                        day = int(parts[2])
                        activity_dates.add(day)
                        activity_map[day] = activity

            except Exception as e:
                print(f"[Display] Error parsing activity: {e}")

        # Create visual representation
        self._render_calendar_grid(activity_dates, activity_map, activities)

    def _render_calendar_grid(self, activity_dates, activity_map, all_activities):
        """
        Render a calendar grid showing which days have activities

        This is a simple implementation that shows:
        - Activity indicator dots for days with activities
        - Date labels
        - Summary stats
        """
        # For now, show a simple summary
        # (Full calendar visualization would need more sophisticated layout)

        try:
            # Show activity summary
            summary = f"Activities: {len(all_activities)}"
            text_area = label.Label(
                self.font or bitmap_font.load_font("/fonts/font5x8.bdf"),
                text=summary,
                color=0x00FF00,
            )
            text_area.x = 5
            text_area.y = 5

            self.group.append(text_area)

            # Show activity types summary
            activity_types = {}
            for activity in all_activities:
                act_type = activity.get("type", "unknown")
                activity_types[act_type] = activity_types.get(act_type, 0) + 1

            # Display each type
            y_pos = 15
            for act_type, count in activity_types.items():
                line = f"{act_type}: {count}"
                text_line = label.Label(
                    self.font or bitmap_font.load_font("/fonts/font5x8.bdf"),
                    text=line,
                    color=0xFFFF00,
                )
                text_line.x = 5
                text_line.y = y_pos
                self.group.append(text_line)
                y_pos += 8

                if y_pos > self.height - 8:
                    break

            print("[Display] Calendar grid rendered")

        except Exception as e:
            print(f"[Display] Error rendering grid: {e}")
            self.show_message("Render Error")

    def set_brightness(self, brightness):
        """
        Set display brightness

        Args:
            brightness (float): Brightness 0.0 to 1.0
        """
        try:
            self.display.brightness = max(0.0, min(1.0, brightness))
        except Exception as e:
            print(f"[Display] Error setting brightness: {e}")
