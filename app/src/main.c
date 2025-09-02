#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "sensors/dht11.h"

#include <zephyr/net/socket.h>
#include "network/wifi.h"

// NOTE: define personal SSID and password for this to work
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define HTTP_HOST "example.com"
#define HTTP_URL "/"

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

// Settings
static const int32_t sleep_time_ms = 1000;
static char http_response[512];

void print_dns_result(struct zsock_addrinfo** results)
{
	char ipv4[INET_ADDRSTRLEN];

	for (struct zsock_addrinfo* addr = *results; addr != NULL; addr = addr->ai_next)
	{
		if (addr->ai_family == AF_INET)
		{
			struct sockaddr_in* ipv4_addr = (struct sockaddr_in*)addr->ai_addr;
			zsock_inet_ntop(AF_INET, &ipv4_addr->sin_addr, ipv4, sizeof(ipv4));
			LOG_INF("IPv4: %s\n", ipv4);
		}
	}
}

int main(void)
{
	struct zsock_addrinfo hints;
    struct zsock_addrinfo *res;
    char http_request[512];
    int sock;
    int len;
    uint32_t rx_total;
    int ret;

    printk("HTTP GET Demo\r\n");

    // Initialize WiFi
    wifi_init();
    printk("Successfully called wifi_init()");

    // Connect to the WiFi network (blocking)
    ret = wifi_connect(WIFI_SSID, WIFI_PASSWORD);
    if (ret < 0) {
        printk("Error (%d): WiFi connection failed\r\n", ret);
        return 0;
    }

    // Wait to receive an IP address (blocking)
    wifi_wait_for_ip_address();

    // Construct HTTP GET request
    snprintf(http_request,
             sizeof(http_request),
             "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n",
             HTTP_URL,
             HTTP_HOST);

    // Clear and set address info
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;              // IPv4
    hints.ai_socktype = SOCK_STREAM;        // TCP socket

    // Perform DNS lookup
    printk("Performing DNS lookup...\r\n");
    ret = zsock_getaddrinfo(HTTP_HOST, "80", &hints, &res);
    if (ret != 0) {
        printk("Error (%d): Could not perform DNS lookup\r\n", ret);
        return 0;
    }

    // Print the results of the DNS lookup
    print_dns_result(&res);

    // Create a new socket
    sock = zsock_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        printk("Error (%d): Could not create socket\r\n", errno);
        return 0;
    }

    // Connect the socket
    ret = zsock_connect(sock, res->ai_addr, res->ai_addrlen);
    if (ret < 0) {
        printk("Error (%d): Could not connect the socket\r\n", errno);
        return 0;
    }

    // Set the request
    printk("Sending HTTP request...\r\n");
    ret = zsock_send(sock, http_request, strlen(http_request), 0);
    if (ret < 0) {
        printk("Error (%d): Could not send request\r\n", errno);
        return 0;
    }

    // Print the response
    printk("Response:\r\n\r\n");
    rx_total = 0;
    while (1) 
	{
        // Receive data from the socket
        len = zsock_recv(sock, http_response, sizeof(http_response) - 1, 0);

        // Check for errors
        if (len < 0) {
            printk("Receive error (%d): %s\r\n", errno, strerror(errno));
            return 0;
        }

        // Check for end of data
        if (len == 0) {
            break;
        }

        // Null-terminate the response string and print it
        http_response[len] = '\0';
        printk("%s", http_response);
        rx_total += len;
    }

    // Print the total number of bytes received
    printk("\r\nTotal bytes received: %u\r\n", rx_total);

    // Close the socket
    zsock_close(sock);

    return 0;
}

// int main(void)
// {
//     const struct gpio_dt_spec power_led = GPIO_DT_SPEC_GET(DT_ALIAS(power_led), gpios);

// 	int led_state = 0;
// 	struct sensor_value temp_value;
// 	struct sensor_value humidity_value;

// 	// Ensure that the peripherals were initialized
// 	__ASSERT(gpio_is_ready_dt(&power_led), "Error initializing LED.\n");

// 	// Set the GPIO as output
// 	if (gpio_pin_configure_dt(&power_led, GPIO_OUTPUT) < 0)
// 	{
// 		LOG_ERR("Error configuring LED pin.\n");
// 	}

// 	dht_checkInit();

// 	while (1)
// 	{
// 		// Toggle the LED
// 		led_state = !led_state;
// 		if (gpio_pin_set_dt(&power_led, led_state) < 0)
// 		{
// 			LOG_ERR("Error setting GPIO pin\n");
// 		}

// 		// Read temperature and humidity
//         dht11_getTemperatureAndHumidity(&temp_value, &humidity_value);
//         LOG_INF("Temperature: %d.%06d C, Humidity: %d.%06d %%\n",
//                temp_value.val1, temp_value.val2,
//                humidity_value.val1, humidity_value.val2);

// 		// Sleep
// 		k_msleep(sleep_time_ms);
// 	}

// 	return 0;
// }