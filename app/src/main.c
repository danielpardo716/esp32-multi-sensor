#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>

// Settings
static const int32_t sleep_time_ms = 1000;
static const struct gpio_dt_spec power_led = GPIO_DT_SPEC_GET(DT_ALIAS(power_led), gpios);
static const struct device* temp_humidity_sensor = DEVICE_DT_GET(DT_ALIAS(temp_humidity_sensor));

int main(void)
{
	int ret;
	int state = 0;

	// Make sure that the GPIO was initialized
	if (!gpio_is_ready_dt(&power_led)) {
		return 0;
	}

	// Set the GPIO as output
	ret = gpio_pin_configure_dt(&power_led, GPIO_OUTPUT);
	if (ret < 0) {
		return 0;
	}

	// Do forever
	while (1) {

		// Change the state of the pin and print
		state = !state;
		printk("LED state: %d\r\n", state);
		
		// Set pin state
		ret = gpio_pin_set_dt(&power_led, state);
		if (ret < 0) {
			return 0;
		}

		// Sleep
		k_msleep(sleep_time_ms);
	}

	return 0;
}
