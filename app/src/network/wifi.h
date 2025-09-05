#ifndef WIFI_H
#define WIFI_H

void wifi_init();
int wifi_connect(const char* ssid, const char* password);
int wifi_disconnect();
void wifi_wait_for_ip_address();
void wifi_print_status();
void wifi_print_available_networks();

#endif