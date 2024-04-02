# _WROOM_WROVER_UART_

## Code Usage
Designed for ESP32 chips, specifically tested on the ESP32-WROOM-based DOIT DevKit v1. Library builds in Windows 10/11 VSCode using 
the ESP-IDF extension. Listed below are the requirements for a good build.

## Build Instructions
Download VSCode, and follow this tutorial to install the ESP-IDF extension: 
https://github.com/espressif/vscode-esp-idf-extension/blob/HEAD/docs/tutorial/install.md 

NOTE: It is important to install system files in a directory with the shortest path length possible (e.g., `C:\Users\{USERNAME}\esp`) to avoid potential build and flash issues.

After this is completed, follow these steps:
- Click on the ESP-IDF button on the left pane to see the Espressif Command List.
- Set the Espressif target as esp32 > ESP32 chip (via USB-Bridge).
- Ensure that the serial port matches the COM port on the machine.
- Build the project using the Espressif Build Command

## Flashing Instructions
After building the project, select the Espressif Flash Command (or Espressif Build, Flash, and Monitor command). If this is the first time
flashing with this library, a prompt will appear asking which interface to flash with: select UART

## Interface Instructions
Flash and power both the chips with code according to the relevant comments. In other words, the WROVER should only be flashed with any code blocks surrounded by comments with the phrase `WROVER_CODE`, and any code blocks with the phrase `WROOM_CODE` should be commented out. The converse is true for flashing the WROOM. This will allow each chip to use the correct UART channel and pins.

Power both chips. Attach a serial monitor or signal debugger to the TX/RX communication channel between the chips to monitor any incoming signals.

Customize the library as you like to view/respond to incoming/outgoing UART events to ensure that bidirectional communication is working.