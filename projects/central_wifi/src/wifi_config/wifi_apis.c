// --- includes ----------------------------------------------------------------
#include "wifi_config.h"

#include <zephyr/net/wifi.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/net_if.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(wifi_apis_m);

// --- static variable definitions ---------------------------------------------
static bool is_wifi_module_initialized = false;

// --- variable definitions ----------------------------------------------------
static struct wifi_connect_req_params cnx_params;
// --- function definitions ----------------------------------------------------
/**
 * @brief Function to initiate wifi connection.
 *
 * @return int
 */
int wifi_apis_connect(void)
{
    if(!is_wifi_module_initialized)
    {
        LOG_ERR("Wifi module not init yet");
        return 0;
    }
    struct net_if *iface = net_if_get_default();
    if(iface == NULL)
    {
        LOG_ERR("Network iface is NULL");
        return 0;
    }

    wifi_config_params(&cnx_params);
    int err = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface,
                 &cnx_params, sizeof(struct wifi_connect_req_params));
    if (err)
    {
        LOG_ERR("Connection request failed %d", err);
        return -ENOEXEC;
    }

    LOG_INF("Connection requested");

    return 0;
}

/**
 * @brief API function to disconnect wifi
 *
 * @return int
 */
int wifi_apis_disconnect(void)
{
    if(!is_wifi_module_initialized)
    {
        LOG_ERR("Wifi module not init yet");
        return 0;
    }

    wifi_config_req_disconnect();

    return 0;
}

/**
 * @brief API function to initialize the WiFi module
 *
 */
void wifi_apis_wifi_init(void)
{
    wifi_config_init();
    is_wifi_module_initialized = true;
}