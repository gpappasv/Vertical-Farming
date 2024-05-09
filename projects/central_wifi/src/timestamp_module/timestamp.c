// --- includes ----------------------------------------------------------------
#include "timestamp.h"
#include <zephyr/net/sntp.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_ip.h>
LOG_MODULE_REGISTER(timestamp_m);
// --- defines -----------------------------------------------------------------
#define SNTP_PORT 123

// --- static variables definitions --------------------------------------------
static int64_t timestamp = 0;

// --- structs -----------------------------------------------------------------
static int sntp(void)
{
    struct sntp_time sntp_time;
    int rv;
    char server[] = "time.google.com";
    rv = sntp_simple(server, 4000, &sntp_time);
    if (rv < 0)
    {
        LOG_ERR("SNTP failed with: %d", rv);
        return -1;
    }

    timestamp = sntp_time.seconds * 1000;
    return 0;
}

/**
 * @brief function to update timestamp
 * 
 */
void update_timestamp(void)
{
    // Get calendar time
    LOG_INF("Attempt to update cloud data timestamp");
    sntp();
    // Add 2 hours to Unix time to match our timezone offset
    // TODO: Summer time is adjusted in database_tools.py
    timestamp += 3600000;
    LOG_INF("The updated timestamp is: %lld", timestamp);
}

/**
 * @brief Get the timestamp object
 * 
 * @return int64_t 
 */
int64_t get_timestamp(void)
{
    return timestamp;
}