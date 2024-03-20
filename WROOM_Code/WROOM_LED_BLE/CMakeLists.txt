# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(COMPONENT_REQUIRES
  "Adafruit_BusIO"
  "Adafruit_GFX"
  "BLE"
  "ESP32_DMA_Matrix"
  "FastLED")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(WROOM_LED_BLE)
