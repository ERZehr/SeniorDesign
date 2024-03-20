# _ble_spp_server_

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

## Interface Instructions
Flash and power the chip, using a serial monitor or signal debugger on TX0/RX0 to see incoming messages. On an Android-based smartphone, download the nRF Connect app: https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp. On the scanner tab, find the device labeled ESP32_BLE and connect to it. There is a tab of operations under "Nordic UART Service", with a UUID matching the code's `SERVICE_UUID`. At the time of prototyping, this UUID has both a TX and RX characteristic, both from the chip's perspective. Click on the upload arrow next to the RX Characteristic, and send any string you like. The resulting string should get sent to the chip, where it will be output to the serial line.