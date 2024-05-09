#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// --- includes ----------------------------------------------------------------
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/wifi_mgmt.h>

// --- functions declarations --------------------------------------------------
void wifi_config_init(void);
void wifi_config_req_disconnect(void);
bool wifi_config_is_wifi_connected(void);
int wifi_config_params(struct wifi_connect_req_params *params);

#endif // WIFI_CONFIG_H
