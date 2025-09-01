#include <zephyr/drivers/sensor.h>

/**
 * @brief Checks if the DHT11 sensor is initialized.
 */
void dht_checkInit();

/**
 * @brief Reads temperature and humidity from the DHT11 sensor.
 *
 * @param temp Pointer to a sensor_value struct to store the temperature.
 * @param humidity Pointer to a sensor_value struct to store the humidity.
 */
void dht11_getTemperatureAndHumidity(struct sensor_value* temp, struct sensor_value* humidity);