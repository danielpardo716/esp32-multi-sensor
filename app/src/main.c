#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "sensors/dht11.h"

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

// Settings
#define SLEEP_TIME_MS 	1000

static const struct device* pLdr_adc = DEVICE_DT_GET(DT_ALIAS(ldr_adc_device));
static const struct adc_channel_cfg ldr_adc_channel = ADC_CHANNEL_CFG_DT(DT_ALIAS(ldr_adc_channel));

int main(void)
{
	const struct gpio_dt_spec power_led = GPIO_DT_SPEC_GET(DT_ALIAS(power_led), gpios);
	int led_state = 0;

	uint16_t adc_buffer;
	struct adc_sequence sequence = {
		.channels = BIT(ldr_adc_channel.channel_id),
		.buffer = &adc_buffer,
		.buffer_size = sizeof(adc_buffer),
		.resolution = DT_PROP(DT_ALIAS(ldr_adc_channel), zephyr_resolution),
	};
	
	struct sensor_value temp_value;
	struct sensor_value humidity_value;

	// Ensure that the peripherals were initialized
	__ASSERT(gpio_is_ready_dt(&power_led), "Error initializing LED.\n");
	__ASSERT(device_is_ready(pLdr_adc), "Failed to get ADC device binding.\n");
	dht_checkInit();

	// Set the GPIO as output
	if (gpio_pin_configure_dt(&power_led, GPIO_OUTPUT) < 0)
	{
		LOG_ERR("Error configuring LED pin.\n");
	}

	// Configure the ADC channel
	if (adc_channel_setup(pLdr_adc, &ldr_adc_channel) < 0)
	{
		LOG_ERR("Failed to configure ADC channel.\n");
	}

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
        // LOG_INF("Temperature: %d.%06d C, Humidity: %d.%06d %%\n",
        //        temp_value.val1, temp_value.val2,
        //        humidity_value.val1, humidity_value.val2);

		// Read LDR value
        if (adc_read(pLdr_adc, &sequence) < 0)
		{
            LOG_ERR("ADC read failed\n");
        }
		else
		{
			static const int32_t vref_mv = DT_PROP(DT_ALIAS(ldr_adc_channel), zephyr_vref_mv);
			int32_t adc_mv_value = (adc_buffer * vref_mv) / (1 << sequence.resolution);
            LOG_INF("LDR raw value: %d, mV: %d", adc_buffer, adc_mv_value);
        }

		// Sleep
		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}