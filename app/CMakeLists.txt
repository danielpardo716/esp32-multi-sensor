# Minimum CMake version
cmake_minimum_required(VERSION 3.20.0)

# Locate the Zephyr RTOS source
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Name the project
project(
    esp32_multi_sensor
    VERSION 0.1.0
    LANGUAGES C
)

# Locate the source code for the application
target_sources(app PRIVATE src/main.c)