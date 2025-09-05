#include "wifi.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/wifi_mgmt.h>

#define MAX_NUMBER_OF_ACCESS_POINTS 10

// Register for logging
LOG_MODULE_REGISTER(wifi, LOG_LEVEL_INF);

// Event callbacks
static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;
static struct net_mgmt_event_callback scan_cb;

// Semaphores
static K_SEM_DEFINE(sem_wifi, 0, 1);
static K_SEM_DEFINE(sem_ipv4, 0, 1);
static K_SEM_DEFINE(sem_scan, 0, 1);

// Callback handlers
static void on_wifi_connection_event(struct net_mgmt_event_callback* cb, uint32_t mgmt_event, struct net_if* iface)
{
    const struct wifi_status* status = (const struct wifi_status*)cb->info;

    if (mgmt_event == NET_EVENT_WIFI_CONNECT_RESULT)
    {
        if (status->status)
        {
            LOG_ERR("Error (%d): Connection request failed.\n", status->status);
        }
        else
        {
            LOG_INF("Wi-Fi connected.\n");
            k_sem_give(&sem_wifi);
        }
    }
    else if (mgmt_event == NET_EVENT_WIFI_DISCONNECT_RESULT)
    {
        if (status->status)
        {
            LOG_ERR("Error (%d): Disconnection request failed.\n", status->status);
        }
        else
        {
            LOG_INF("Wi-Fi disconnected.\n");
            k_sem_take(&sem_wifi, K_NO_WAIT);
        }
    }
}

static void on_ipv4_obtained_event(struct net_mgmt_event_callback* cb, uint32_t mgmt_event, struct net_if* iface)
{
    if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD)
    {
        LOG_INF("IPv4 address assigned\r\n");
        k_sem_give(&sem_ipv4);
    }
}

static struct wifi_scan_result scan_results[MAX_NUMBER_OF_ACCESS_POINTS];
static int scan_count = 0;
static void on_wifi_scan_event(struct net_mgmt_event_callback* cb, uint32_t mgmt_event, struct net_if* iface)
{
    if (mgmt_event == NET_EVENT_WIFI_SCAN_RESULT)
    {
        struct wifi_scan_result* result = (struct wifi_scan_result*)cb->info;
        if (scan_count < MAX_NUMBER_OF_ACCESS_POINTS)
        {
            memcpy(&scan_results[scan_count++], result, sizeof(*result));
        }
    }
    else if (mgmt_event == NET_EVENT_WIFI_SCAN_DONE)
    {
        LOG_INF("Wi-Fi scan completed.\n");
        k_sem_give(&sem_scan);
    }
}

void wifi_init()
{
    // Initialize event callbacks
    net_mgmt_init_event_callback(&wifi_cb, on_wifi_connection_event, NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);
    net_mgmt_init_event_callback(&ipv4_cb, on_ipv4_obtained_event, NET_EVENT_IPV4_ADDR_ADD);
    net_mgmt_init_event_callback(&scan_cb, on_wifi_scan_event, NET_EVENT_WIFI_SCAN_RESULT | NET_EVENT_WIFI_SCAN_DONE);
    net_mgmt_add_event_callback(&wifi_cb);
    net_mgmt_add_event_callback(&ipv4_cb);
    net_mgmt_add_event_callback(&scan_cb);
}

int wifi_connect(const char* ssid, const char* password)
{
    struct net_if* iface = net_if_get_default();
    struct wifi_connect_req_params params = {
        .ssid = ssid,
        .ssid_length = strlen(ssid),
        .psk = password,
        .psk_length = strlen(password),
        .security = WIFI_SECURITY_TYPE_PSK,
        .band = WIFI_FREQ_BAND_2_4_GHZ,
        .channel = WIFI_CHANNEL_ANY,
        .mfp = WIFI_MFP_OPTIONAL,
        .timeout = SYS_FOREVER_MS,
    };

    int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params));
    if (ret)
    {
        LOG_ERR("Error (%d): Connection request failed\r\n", ret);
        return ret;
    }

    // Wait for connection event
    k_sem_take(&sem_wifi, K_SECONDS(CONFIG_NET_CONFIG_INIT_TIMEOUT));
    return ret;
}

int wifi_disconnect()
{
    struct net_if* iface = net_if_get_default();
    return net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface, NULL, 0);
}

static char ip_address[NET_IPV4_ADDR_LEN];
static char gw_address[NET_IPV4_ADDR_LEN];
void wifi_wait_for_ip_address()
{
    struct net_if* iface = net_if_get_default();

    k_sem_take(&sem_ipv4, K_SECONDS(CONFIG_NET_CONFIG_INIT_TIMEOUT));

    // Get the IP address
    memset(ip_address, 0, sizeof(ip_address));
    if (net_addr_ntop(AF_INET,
                      &iface->config.ip.ipv4->unicast[0].ipv4.address.in_addr,
                      ip_address,
                      sizeof(ip_address)) == NULL)
    {
        LOG_ERR("Error: Could not convert IP address to string\r\n");
    }

    // Get the gateway address
    memset(gw_address, 0, sizeof(gw_address));
    if (net_addr_ntop(AF_INET,
                      &iface->config.ip.ipv4->gw,
                      gw_address,
                      sizeof(gw_address)) == NULL) {
        LOG_ERR("Error: Could not convert gateway address to string\r\n");
    }
}

void wifi_print_status()
{
    // Get WiFi status
    struct wifi_iface_status status;
    if (net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, net_if_get_default(), &status, sizeof(struct wifi_iface_status)) != 0)
    {
        LOG_ERR("Error: WiFi status request failed\r\n");
    }

    // Print the WiFi status
    printk("WiFi status:\r\n");
    if (status.state >= WIFI_STATE_ASSOCIATED) {
        printk("  SSID: %-32s\r\n", status.ssid);
        printk("  Band: %s\r\n", wifi_band_txt(status.band));
        printk("  Channel: %d\r\n", status.channel);
        printk("  Security: %s\r\n", wifi_security_txt(status.security));
        printk("  IP address: %s\r\n", ip_address);
        printk("  Gateway: %s\r\n", gw_address);
    }
}

void wifi_print_available_networks()
{
    LOG_INF("Scanning for Wi-Fi networks...\n");
    if (net_mgmt(NET_REQUEST_WIFI_SCAN, net_if_get_default(), NULL, 0))
    {
        LOG_ERR("Scan request failed\n");
        return;
    }

    // Reset from previous scan
    memset(scan_results, 0, sizeof(scan_results));
    scan_count = 0;

    // Wait for scan to complete
    k_sem_take(&sem_scan, K_SECONDS(CONFIG_NET_CONFIG_INIT_TIMEOUT));
    
    // Print results
    for (int i = 0; i < scan_count; i++) 
    {
        printk("[%d] SSID: %-32s\r\n    RSSI: %d dBm, Security: %s\n", i,
               scan_results[i].ssid,
               scan_results[i].rssi,
               scan_results[i].security == WIFI_SECURITY_TYPE_PSK ? "WPA/WPA2" :
               scan_results[i].security == WIFI_SECURITY_TYPE_NONE ? "Open" : "Other"
        );
    }
}