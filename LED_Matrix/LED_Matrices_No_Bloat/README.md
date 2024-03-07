# _LED_Matrices_No_Bloat_

## Code Usage
Designed for ESP32 chips, specifically tested on the ESP32-WROOM-based DOIT DevKit v1. Libary builds in Windows 10/11 VSCode using 
the ESP-IDF extension. Listed below are the requirements for a good build.

## Build Instructions
Download VSCode, and follow this tutorial to install the ESP-IDF extension: 
https://github.com/espressif/vscode-esp-idf-extension/blob/HEAD/docs/tutorial/install.md After this is completed, follow these steps:
- Click on the ESP-IDF button on the left pane to see the Espressif Command List.
- Set the Espressif target as esp32 > ESP32 chip (via USB-Bridge).
- Ensure that the serial port matches the COM port on the machine.
- Build the project using the Espressif Build Command

## Flashing Instructions
After building the project, select the Espressif Flash Command (or Espressif Build, Flash, and Monitor command). If this is the first time
flashing with this library, a prompt will appear asking which interface to flash with: select UART

## Hardware Instructions
Follow the pin mappings in [main](main/main.cpp) (GPIO numbers, not hardware pins) to connect ESP32 pins to HUB75 matrix pins.
