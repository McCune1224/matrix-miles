# Archived Documentation

This directory contains documentation and code from previous iterations of the Matrix Miles project.

## Contents

### MicroPython/Python ESP32 Client (Archived)

These files are from an earlier version of the project that used MicroPython on a generic ESP32 board:

- `ESP32_SETUP_GUIDE.md` - MicroPython setup guide
- `micropython-setup.md` - MicroPython installation instructions
- `micropython-setup-old.md` - Earlier version of MicroPython setup

The Python/MicroPython client code is still in the repository at:
- `../esp32_client_python/` - Python client implementation

### C Proof of Concept (Archived)

The C proof-of-concept code is in:
- `../c-proof-of-concept/` - Early C implementation

## Current Implementation

The project has moved to:
- **Hardware:** Arduino Nano ESP32 (ESP32-S3)
- **Language:** C++ with Arduino framework
- **Tools:** arduino-cli and Neovim

See the main documentation:
- `../README.md` - Main project documentation
- `../ARDUINO_NANO_ESP32_SETUP.md` - Current hardware setup
- `../CPP_CLIENT_GUIDE.md` - Current C++ development guide
- `../NEOVIM_SETUP.md` - Development environment setup

## Why the Change?

1. **Better Hardware:** Arduino Nano ESP32 has ESP32-S3 chip with better performance
2. **Native C++:** Better performance and memory management than MicroPython
3. **Better Tooling:** arduino-cli provides excellent command-line workflow
4. **LSP Support:** Full IDE features with clangd in Neovim
5. **Library Ecosystem:** Access to entire Arduino library ecosystem

## Historical Note

These archived files are kept for reference only and are no longer maintained.
