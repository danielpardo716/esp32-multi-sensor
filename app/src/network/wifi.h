#ifndef WIFI_H
#define WIFI_H

void wifi_init();
int wifi_connect(char* ssid, char* password);
void wifi_wait_for_ip_address();
int wifi_disconnect();

#endif