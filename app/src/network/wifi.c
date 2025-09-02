#include "wifi.h"

#include <zephyr/kernel.h>
#include <zephyr/net/wifi_mgmt.h>

// Event callbacks
static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;

// Semaphores
static K_SEM_DEFINE(sem_wifi, 0, 1);
static K_SEM_DEFINE(sem_ipv4, 0, 1);

// Callback handlers
static void on_wifi_connection_event(struct net_mgmt_event_callback* cb, uint32_t mgmt_event, struct net_if* iface)
{
    const struct wifi_status* status = (const struct wifi_status*)cb->info;

    if (mgmt_event == NET_EVENT_WIFI_CONNECT_RESULT)
    {
        if (status->status)
        {
            printk("Error (%d): Connection request failed\r\n", status->status);
        }
        else
        {
            printk("Connected!\r\n");
            k_sem_give(&sem_wifi);
        }
    }
    else if (mgmt_event == NET_EVENT_WIFI_DISCONNECT_RESULT)
    {
        if (status->status)
        {
            printk("Error (%d): Disconnection request failed\r\n", status->status);
        }
        else
        {
            printk("Disconnected\r\n");
            k_sem_take(&sem_wifi, K_NO_WAIT);
        }
    }
}

static void on_ipv4_obtained_event(struct net_mgmt_event_callback* cb, uint32_t mgmt_event, struct net_if* iface)
{
    if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD)
    {
        printk("IPv4 address assigned\r\n");
        k_sem_give(&sem_ipv4);
    }
}

void wifi_init()
{
    // Initialize event callbacks
    net_mgmt_init_event_callback(&wifi_cb, on_wifi_connection_event, NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);
    net_mgmt_init_event_callback(&ipv4_cb, on_ipv4_obtained_event, NET_EVENT_IPV4_ADDR_ADD);
    net_mgmt_add_event_callback(&wifi_cb);
    net_mgmt_add_event_callback(&ipv4_cb);
}

int wifi_connect(char* ssid, char* password)
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
        printk("Error (%d): Connection request failed\r\n", ret);
        return ret;
    }

    // Wait for connection event
    k_sem_take(&sem_wifi, K_FOREVER);
    return ret;
}

void wifi_wait_for_ip_address()
{
    struct net_if* iface = net_if_get_default();
    struct wifi_iface_status status;
    char ip_address[NET_IPV4_ADDR_LEN];
    char gw_address[NET_IPV4_ADDR_LEN];

    k_sem_take(&sem_ipv4, K_FOREVER);

    // Get WiFi status
    if (net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status, sizeof(struct wifi_iface_status)) != 0)
    {
        printk("Error: WiFi status request failed\r\n");
    }

    // Get the IP address
    memset(ip_address, 0, sizeof(ip_address));
    if (net_addr_ntop(AF_INET,
                      &iface->config.ip.ipv4->unicast[0].ipv4.address.in_addr,
                      ip_address,
                      sizeof(ip_address)) == NULL)
    {
        printk("Error: Could not convert IP address to string\r\n");
    }

    // Get the gateway address
    memset(gw_address, 0, sizeof(gw_address));
    if (net_addr_ntop(AF_INET,
                      &iface->config.ip.ipv4->gw,
                      gw_address,
                      sizeof(gw_address)) == NULL) {
        printk("Error: Could not convert gateway address to string\r\n");
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

int wifi_disconnect()
{
    struct net_if* iface = net_if_get_default();
    return net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface, NULL, 0);
}
