// --- includes ----------------------------------------------------------------
#include <zephyr/net/wifi.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/net_if.h>
#include <zephyr/logging/log.h>
#include <net_private.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(wifi_config_m);

// --- defines -----------------------------------------------------------------
#define WIFI_SSID "Ziggo-net5G"
#define WIFI_PASS "19981998Gg"
#define WIFI_SEC_WPA2 WIFI_SECURITY_TYPE_PSK
#define WIFI_MGMT_EVENTS (NET_EVENT_WIFI_CONNECT_RESULT | \
                          NET_EVENT_WIFI_DISCONNECT_RESULT)
K_SEM_DEFINE(wait_for_next, 0, 1);

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define WIFI_LED_FEEDBACK_WORK_SCHEDULE_PERIOD_INIT_MS 750
#define WIFI_LED_FEEDBACK_WORK_SCHEDULE_PERIOD_DISCONNECTED_MS 250
#define WIFI_LED_FEEDBACK_WORK_SCHEDULE_PERIOD_CONNECTED_MS 1000

// --- static variables definitions --------------------------------------------
struct wifi_iface_status status = {0};
static struct net_mgmt_event_callback wifi_sta_mgmt_cb;
static struct net_mgmt_event_callback net_addr_mgmt_cb;
static struct
{
    uint8_t connected : 1;
    uint8_t disconnect_requested : 1;
    uint8_t first_init: 1;
    uint8_t ipv6_addr_dhcp_init: 1;
    uint8_t ipv4_addr_dhcp_init: 1;
    uint8_t _unused : 3;
} context;
struct k_work_delayable wifi_led_fb_work;
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);

// --- static function declarations --------------------------------------------
static int cmd_wifi_status(void);
static void handle_wifi_connect_result(struct net_mgmt_event_callback *cb);
static void handle_wifi_disconnect_result(struct net_mgmt_event_callback *cb);
static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                    uint32_t mgmt_event, struct net_if *iface);
static void print_dhcpv4_ip(struct net_mgmt_event_callback *cb);
static void net_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                   uint32_t mgmt_event, struct net_if *iface);

// --- static function definitions ---------------------------------------------
static int cmd_wifi_status(void)
{
    struct net_if *iface = net_if_get_default();

    if (net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status,
                 sizeof(struct wifi_iface_status)))
    {
        LOG_INF("Status request failed\n");

        return -ENOEXEC;
    }

    LOG_INF("Status: successful\n");
    LOG_INF("==================\n");
    LOG_INF("State: %s\n", wifi_state_txt(status.state));

    if (status.state >= WIFI_STATE_ASSOCIATED)
    {
        uint8_t mac_string_buf[sizeof("xx:xx:xx:xx:xx:xx")];

        LOG_INF("Interface Mode: %s",
                wifi_mode_txt(status.iface_mode));
        LOG_INF("Link Mode: %s",
                wifi_link_mode_txt(status.link_mode));
        LOG_INF("SSID: %.32s", status.ssid);
        LOG_INF("BSSID: %s",
                net_sprint_ll_addr_buf(
                    status.bssid, WIFI_MAC_ADDR_LEN,
                    mac_string_buf, sizeof(mac_string_buf)));
        LOG_INF("Band: %s", wifi_band_txt(status.band));
        LOG_INF("Channel: %d", status.channel);
        LOG_INF("Security: %s", wifi_security_txt(status.security));
        LOG_INF("MFP: %s", wifi_mfp_txt(status.mfp));
        LOG_INF("RSSI: %d", status.rssi);
    }

    return 0;
}

static void handle_wifi_connect_result(struct net_mgmt_event_callback *cb)
{
    const struct wifi_status *status =
        (const struct wifi_status *)cb->info;

    if (status->status)
    {
        LOG_ERR("Connection request failed (%d)", status->status);
    }
    else
    {
        LOG_INF("Connected");
        context.connected = true;
        context.first_init = false;
    }

    k_sem_give(&wait_for_next);
    k_work_reschedule(&wifi_led_fb_work, K_NO_WAIT);
}

static void handle_wifi_disconnect_result(struct net_mgmt_event_callback *cb)
{
    const struct wifi_status *status =
        (const struct wifi_status *)cb->info;

    if (context.disconnect_requested)
    {
        LOG_INF("Disconnection request %s (%d)",
                status->status ? "failed" : "done",
                status->status);
        context.disconnect_requested = false;
    }
    else
    {
        LOG_INF("Disconnected");
        context.connected = false;
    }

    cmd_wifi_status();
    k_work_reschedule(&wifi_led_fb_work, K_NO_WAIT);
}

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                    uint32_t mgmt_event, struct net_if *iface)
{
    switch (mgmt_event)
    {
    case NET_EVENT_WIFI_CONNECT_RESULT:
        handle_wifi_connect_result(cb);
        break;
    case NET_EVENT_WIFI_DISCONNECT_RESULT:
        handle_wifi_disconnect_result(cb);
        break;
    default:
        break;
    }
}

static void print_dhcpv6_ip(struct net_mgmt_event_callback *cb)
{
    /* Get DHCP info from struct net_if_dhcpv4 and print */
    const struct net_if_dhcpv6 *dhcpv6 = cb->info;
    const struct in6_addr *addr = &dhcpv6->addr;
    char dhcp_info[128];

    net_addr_ntop(AF_INET6, addr, dhcp_info, sizeof(dhcp_info));

    LOG_INF("IPv6 address: %s", dhcp_info);
    context.ipv6_addr_dhcp_init = true;
    cmd_wifi_status();
    k_sem_give(&wait_for_next);
}

static void print_dhcpv4_ip(struct net_mgmt_event_callback *cb)
{
    /* Get DHCP info from struct net_if_dhcpv6 and print */
    const struct net_if_dhcpv4 *dhcpv4 = cb->info;
    const struct in_addr *addr = &dhcpv4->requested_ip;
    char dhcp_info[128];

    net_addr_ntop(AF_INET, addr, dhcp_info, sizeof(dhcp_info));

    LOG_INF("IPv4 address: %s", dhcp_info);
    context.ipv4_addr_dhcp_init = true;
    cmd_wifi_status();
    k_sem_give(&wait_for_next);
}

static void net_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                   uint32_t mgmt_event, struct net_if *iface)
{
    switch (mgmt_event)
    {
    case NET_EVENT_IPV4_DHCP_BOUND:
        print_dhcpv4_ip(cb);
        break;
    case NET_EVENT_IPV6_DHCP_BOUND:
        print_dhcpv6_ip(cb);
        break;
    default:
        break;
    }
}

void wifi_led_fb_work_handler(struct k_work *work)
{
    if(context.first_init == true)
    {
        gpio_pin_toggle_dt(&led);
        gpio_pin_set_dt(&led1, false);
        k_work_reschedule(&wifi_led_fb_work, K_MSEC(WIFI_LED_FEEDBACK_WORK_SCHEDULE_PERIOD_INIT_MS));
        return;
    }

    if(context.connected == true) {
        gpio_pin_set_dt(&led, true);
        k_work_reschedule(&wifi_led_fb_work, K_MSEC(WIFI_LED_FEEDBACK_WORK_SCHEDULE_PERIOD_CONNECTED_MS));
        if(context.ipv4_addr_dhcp_init && context.ipv6_addr_dhcp_init)
        {
            gpio_pin_set_dt(&led1, true);
        }
        else
        {
            gpio_pin_toggle_dt(&led1);
        }
    }

    if(context.connected == false)
    {
        gpio_pin_toggle_dt(&led);
        gpio_pin_set_dt(&led1, false);
        k_work_reschedule(&wifi_led_fb_work, K_MSEC(WIFI_LED_FEEDBACK_WORK_SCHEDULE_PERIOD_DISCONNECTED_MS));
    }
}

static void init_wifi_led_feedback(void)
{
	if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&led1)) {
		LOG_ERR("WiFi led feedback init failure! Skipping...");
        return;
	}
    int ret = 0;
    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
        LOG_ERR("WiFi led feedback init failure! Skipping...");
		return;
	}
    ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
        LOG_ERR("WiFi led1 feedback init failure! Skipping...");
		return;
	}
    LOG_INF("Initializing WiFi feedback LED");

    k_work_init_delayable(&wifi_led_fb_work, wifi_led_fb_work_handler);
    k_work_schedule(&wifi_led_fb_work, K_NO_WAIT);
}

// --- function definitions ----------------------------------------------------
/**
 * @brief Function to register a disconnect request
 *
 */
void wifi_config_req_disconnect(void)
{
    struct net_if *iface = net_if_get_default();
    if(iface == NULL)
    {
        LOG_ERR("Network iface is NULL");
        return;
    }
    int status;

    context.disconnect_requested = true;

    status = net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface, NULL, 0);

    if (status)
    {
        context.disconnect_requested = false;

        if (status == -EALREADY)
        {
            LOG_INF("Already disconnected");
        }
        else
        {
            LOG_ERR("Disconnect request failed");
            return; //-ENOEXEC;
        }
    }
    else
    {
        LOG_INF("Disconnect requested");
    }
}

int wifi_config_params(struct wifi_connect_req_params *params)
{
    params->timeout = SYS_FOREVER_MS;

    /* Defaults */
    params->band = WIFI_FREQ_BAND_UNKNOWN;
    params->channel = WIFI_CHANNEL_ANY;
    params->security = WIFI_SEC_WPA2;
    params->mfp = WIFI_MFP_OPTIONAL;

    /* SSID */
    params->ssid = WIFI_SSID;
    params->ssid_length = strlen(params->ssid);

    params->psk = WIFI_PASS;
    params->psk_length = strlen(params->psk);

    return 0;
}

bool wifi_config_is_wifi_connected(void)
{
    return (context.ipv4_addr_dhcp_init && context.ipv6_addr_dhcp_init && context.connected);
}

/**
 * @brief Setup wifi configuration. Should be called by the relevant wifi_init api
 *
 */
void wifi_config_init(void)
{
    memset(&context, 0, sizeof(context));
    context.first_init = true;

    net_mgmt_init_event_callback(&wifi_sta_mgmt_cb,
                                 wifi_mgmt_event_handler,
                                 WIFI_MGMT_EVENTS);

    net_mgmt_add_event_callback(&wifi_sta_mgmt_cb);

    net_mgmt_init_event_callback(&net_addr_mgmt_cb,
                                 net_mgmt_event_handler,
                                 NET_EVENT_IPV4_DHCP_BOUND | NET_EVENT_IPV6_DHCP_BOUND);

    net_mgmt_add_event_callback(&net_addr_mgmt_cb);
    init_wifi_led_feedback();
    k_sleep(K_SECONDS(3));
}