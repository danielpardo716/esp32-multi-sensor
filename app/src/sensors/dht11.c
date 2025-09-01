#include "dht11.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(DHT11, LOG_LEVEL_INF);

static const struct device* pSensor = DEVICE_DT_GET(DT_ALIAS(temp_humidity_sensor));

void dht_checkInit()
{
    // Ensure that the sensor device is ready
    __ASSERT(device_is_ready(temp_humidity_sensor), "Error initializing temperature sensor.\n");
}

void dht11_getTemperatureAndHumidity(struct sensor_value* pTempValue, struct sensor_value* pHumidityValue)
{
	// Fetch new data from the sensor
	int result = sensor_sample_fetch(pSensor);
	if (result < 0)
	{
		LOG_ERR("Error fetching sensor data: %d\n", result);
	}

	// Read temperature and humidity channels
	result = sensor_channel_get(pSensor, SENSOR_CHAN_AMBIENT_TEMP, pTempValue);
	if (result < 0)
	{
		LOG_ERR("Error reading temperature: %d\n", result);
	}
    result = sensor_channel_get(pSensor, SENSOR_CHAN_HUMIDITY, pHumidityValue);
    if (result < 0)
    {
        LOG_ERR("Error reading humidity: %d\n", result);
    }
}