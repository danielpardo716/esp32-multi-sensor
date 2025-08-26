#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>

// Settings
static const int32_t sleep_time_ms = 1000;

int main(void)
{
    const struct gpio_dt_spec power_led = GPIO_DT_SPEC_GET(DT_ALIAS(power_led), gpios);
    const struct device* temp_humidity_sensor = DEVICE_DT_GET(DT_ALIAS(temp_humidity_sensor));

	int led_state = 0;
	struct sensor_value temp_value;
	struct sensor_value humidity_value;

	// Make sure that the GPIO was initialized
	if (!gpio_is_ready_dt(&power_led))
	{
		printk("Error initializing LED.\n");
		return 0;
	}

	// Set the GPIO as output
	if (gpio_pin_configure_dt(&power_led, GPIO_OUTPUT) < 0)
	{
		printk("Error configuring LED pin.\n");
		return 0;
	}

	// Initialize the temperature and humidity sensor
	if (!device_is_ready(temp_humidity_sensor))
	{
		printk("Error initializing temperature sensor.\n");
		return 0;
	}

	while (1)
	{
		// Toggle the LED
		led_state = !led_state;
		if (gpio_pin_set_dt(&power_led, led_state) < 0)
		{
			printk("Error setting GPIO pin\n");
		}

		// Read temperature and humidity
        get_temperature_and_humidity(temp_humidity_sensor, &temp_value, &humidity_value);
        printk("Temperature: %d.%06d C, Humidity: %d.%06d %%\n",
               temp_value.val1, temp_value.val2,
               humidity_value.val1, humidity_value.val2);

		// Sleep
		k_msleep(sleep_time_ms);
	}

	return 0;
}

void get_temperature_and_humidity(struct device* pSensor, struct sensor_value* pTempValue, struct sensor_value* pHumidityValue)
{
	// Fetch new data from the sensor
	int result = sensor_sample_fetch(pSensor);
	if (result < 0)
	{
		printk("Error fetching sensor data: %d\n", result);
	}

	// Read temperature and humidity channels
	result = sensor_channel_get(pSensor, SENSOR_CHAN_AMBIENT_TEMP, pTempValue);
	if (result < 0)
	{
		printk("Error reading temperature: %d\n", result);
	}
    result = sensor_channel_get(pSensor, SENSOR_CHAN_HUMIDITY, pHumidityValue);
    if (result < 0)
    {
        printk("Error reading humidity: %d\n", result);
    }
}