# ESP32 Multi-Sensor
This repository contains the code and documentation for a multi-sensor to be used with Home Assistant. The sensor records data (temperature, humidity, illuminance, and motion) every 5 minutes, and reports these values to the self-hosted Home Assistant instance using MQTT.

## Hardware
- ESP32 (38-Pin)
- DHT11 Temperature & Humidity Sensor
- DSN-FIR800 / HC-SR501 PIR Motion Sensor
- DIYables LDR Light Sensor

## Software
This project uses the Zephyr framework.

### Building the Project
Using VSCode's Dev Containers extension, open the project in the supplied dev container. 

For the USB device to be visible inside the container, [usbipd](https://github.com/dorssel/usbipd-win) needs to be used. Install the tool, and in a Windows terminal enter these commands:
```
usbipd list
usbipd bind --busid=<BUSID>
usbipd attach --wsl --busid=<BUSID>
```
<BUSID> is found using the first command. The attach command needs to be run each time the ESP32 is plugged into the computer. Once this is done, run ```ls -l /dev/ttyUSB*``` in the container to verify there is a visible port.

The script ```run.sh``` is provided to build the code, flash the device, and start a serial monitor.

## Future Improvements
- Use collected data to perform automated tasks (turn lights on/off, open/close shades, turn fan on/off, etc.).
- Create streamlined hardware including:
    - Custom PCB
    - 3D-printed case
    - Coin battery power circuit

## References
- [Zephyr Docs](docs.zephyrproject.org/latest/index.html)
- [Shawn Hymel's Introduction to Zephyr](https://github.com/ShawnHymel/introduction-to-zephyr/tree/main)
- [Memfault Practical Zephyr](https://interrupt.memfault.com/tags#practical-zephyr-series)