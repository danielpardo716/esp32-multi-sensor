#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "sensors/dht11.h"

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

// Settings
static const int32_t sleep_time_ms = 1000;

int main(void)
{
    const struct gpio_dt_spec power_led = GPIO_DT_SPEC_GET(DT_ALIAS(power_led), gpios);

	int led_state = 0;
	struct sensor_value temp_value;
	struct sensor_value humidity_value;

	// Ensure that the peripherals were initialized
	__ASSERT(gpio_is_ready_dt(&power_led), "Error initializing LED.\n");

	// Set the GPIO as output
	if (gpio_pin_configure_dt(&power_led, GPIO_OUTPUT) < 0)
	{
		LOG_ERR("Error configuring LED pin.\n");
	}

	dht_checkInit();

	while (1)
	{
		// Toggle the LED
		led_state = !led_state;
		if (gpio_pin_set_dt(&power_led, led_state) < 0)
		{
			LOG_ERR("Error setting GPIO pin\n");
		}

		// Read temperature and humidity
        dht11_getTemperatureAndHumidity(&temp_value, &humidity_value);
        LOG_INF("Temperature: %d.%06d C, Humidity: %d.%06d %%\n",
               temp_value.val1, temp_value.val2,
               humidity_value.val1, humidity_value.val2);

		// Sleep
		k_msleep(sleep_time_ms);
	}

	return 0;
}