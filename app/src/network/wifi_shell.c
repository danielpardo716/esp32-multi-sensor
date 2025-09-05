#include "wifi.h"
#include <zephyr/shell/shell.h>

static int cmd_wifi_scan(const struct shell* shell, size_t argc, char** argv)
{
    wifi_print_available_networks();
    return 0;
}

static int cmd_wifi_status(const struct shell* shell, size_t argc, char** argv)
{
    wifi_print_status();
    return 0;
}

static int cmd_wifi_connect(const struct shell* shell, size_t argc, char** argv)
{
    if (argc != 3)
    {
        shell_print(shell, "Usage: wifi_connect <SSID> <PASSWORD>");
        return -1;
    }

    const char* ssid = argv[1];
    const char* password = argv[2];
    
    wifi_connect(ssid, password);
    return 0;
}

static int cmd_wifi_disconnect(const struct shell* shell, size_t argc, char** argv)
{
    wifi_disconnect();
    return 0;
}

SHELL_CMD_REGISTER(wifi_scan, NULL, "Scan for available Wi-Fi networks", cmd_wifi_scan);
SHELL_CMD_REGISTER(wifi_status, NULL, "Print current Wi-Fi status", cmd_wifi_status);
SHELL_CMD_REGISTER(wifi_connect, NULL, "Connect to a Wi-Fi network: wifi_connect <SSID> <PASSWORD>", cmd_wifi_connect);
SHELL_CMD_REGISTER(wifi_disconnect, NULL, "Disconnect from the current Wi-Fi network", cmd_wifi_disconnect);