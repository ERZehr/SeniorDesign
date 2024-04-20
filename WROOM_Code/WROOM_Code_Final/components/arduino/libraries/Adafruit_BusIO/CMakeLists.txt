# Adafruit Bus IO Library
# https://github.com/adafruit/Adafruit_BusIO
# MIT License

cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "src/Adafruit_I2CDevice.cpp" "src/Adafruit_BusIO_Register.cpp" "src/Adafruit_SPIDevice.cpp" 
                       INCLUDE_DIRS "."
                       REQUIRES arduino)

project(Adafruit_BusIO)
