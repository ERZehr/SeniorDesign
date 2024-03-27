# _BT_DSP_THREAD_

## Code Usage
Designed for ESP32 chips, specifically tested on the ESP32-WROVER-based DOIT DevKitC v4. Library builds in Windows 10/11 VSCode using 
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

## Interface Instructions
Flash and power the chip, using a serial monitor or signal debugger on TX0/RX0 to see logging messages about audio streaming. On an Android-based smartphone, use the built-in Bluetooth device connector to find the device named `ARTYISYN` and pair with it to connect. Begin streaming audio via any media app you desire, including but not limited to: Spotify, YouTube, YouTube Music, Samsung Music, etc. You will see the logging messages on the serial terminal/debugger print out messages about audio streaming, especially related to play/pause and positional information.