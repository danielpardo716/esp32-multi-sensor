#!/bin/bash
clear
cd app
west build -p always -b esp32_devkitc_wroom/esp32/procpu -- -DDTC_OVERLAY_FILE=boards/esp32_devkitc.overlay
west flash
west espressif monitor