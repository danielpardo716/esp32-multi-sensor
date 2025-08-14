# ESP32 Multi-Sensor
This repository contains the code and documentation for a multi-sensor to be used with Home Assistant. The sensor records data (temperature, humidity, illuminance, and motion) every 5 minutes, and reports these values to the self-hosted Home Assistant instance using MQTT.

## Hardware
- ESP32 (38-Pin)
- DHT11 Temperature & Humidity Sensor
- HC-SR501 PIR Motion Sensor
- DIYables LDR Light Sensor

## Software
This project uses the Zephyr framework

### Building the Project
TODO

## Future Improvements
- Use collected data to perform automated tasks (turn lights on/off, open/close shades, turn fan on/off, etc.).
- Create streamlined hardware including:
    - Custom PCB
    - 3D-printed case
    - Coin battery power circuit

## References
- [Zephyr Docs](docs.zephyrproject.org/latest/index.html)