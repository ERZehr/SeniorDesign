# ESP32 Libraries

NOTE: This is a heavily-modified wrapper around the ESP-IDF-accepted `components\arduino\` submodule (https://github.com/espressif/arduino-esp32) in order to make use of its useful `libraries\` build structure. The libraries in arduino-esp32 are <ins>NOT</ins> all in this directory, nor vice-versa. Modules common to both this directory and arduino-esp32 are not tracked by git and will not receive updates. A modified version of the original README follows below:

arduino-esp32 includes libraries for Arduino compatibility along with some object wrappers around hardware specific devices.  Examples are included in the examples folder under each library folder.  The ESP32 includes additional examples which need no special drivers.

### Adafruit_BusIO
  Adafruit-provided code to allow for efficient API-like constructs for Shift Registers, I2C, and SPI

### Adafruit_GFX
  Adafruit-provided graphics library, useful for output to LED Matrix, OLED, and more

### BLE
  Bluetooth Low Energy v4.2 client/server framework

### ESP32_DMA_Matrix
  Functions for chaining and virtualization of HUB75-style LED Matrices on ESP32 chips

### FastLED
  Library for easily and efficiently controlling a wide variety of LED chipsets

### SimpleBLE
  Minimal BLE advertiser

### SPI
  Arduino compatible Serial Peripheral Interface driver (master only)

### Wire
  Arduino compatible I2C driver
