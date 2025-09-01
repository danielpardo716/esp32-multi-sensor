#include "ldr.h"
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(LDR, LOG_LEVEL_INF);

static const struct device* pLdr_adc = DEVICE_DT_GET(DT_ALIAS(ldr_adc_device));
static const struct adc_channel_cfg ldr_adc_channel = ADC_CHANNEL_CFG_DT(DT_ALIAS(ldr_adc_channel));
static uint16_t adc_buffer;
static struct adc_sequence adc_sequence = {
    .channels = BIT(ldr_adc_channel.channel_id),
    .buffer = &adc_buffer,
    .buffer_size = sizeof(adc_buffer),
    .resolution = DT_PROP(DT_ALIAS(ldr_adc_channel), zephyr_resolution),
};

void ldr_init()
{
    // Ensure that the ADC device is ready
    __ASSERT(device_is_ready(pLdr_adc), "Failed to get ADC device binding.\n");

    // Configure the ADC channel
	if (adc_channel_setup(pLdr_adc, &ldr_adc_channel) < 0)
	{
		LOG_ERR("Failed to configure ADC channel.\n");
	}
}

uint16_t ldr_read()
{
    // Read the ADC value
    if (adc_read(pLdr_adc, &adc_sequence) < 0)
    {
        LOG_ERR("Failed to read ADC value\n");
        return 0;
    }
    else
    {
        // TODO: determine ADC to lux conversion
        // Convert the raw ADC value to millivolts
        static const int32_t vref_mv = DT_PROP(DT_ALIAS(ldr_adc_channel), zephyr_vref_mv);
        return (adc_buffer * vref_mv) / (1 << adc_sequence.resolution);
    }
}