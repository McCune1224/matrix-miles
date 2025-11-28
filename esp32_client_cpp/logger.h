#ifndef LOGGER_H
#define LOGGER_H

#define SERIAL Serial

class Logger {
public:
  Logger() {}

  static void info(const char *message) {
    SERIAL.print("[INFO] ");
    SERIAL.println(message);
  }

  static void error(const char *message) {
    SERIAL.print("[ERROR] ");
    SERIAL.println(message);
  }
}

#endif // LOGGER_H
