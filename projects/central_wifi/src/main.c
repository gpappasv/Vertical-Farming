#include <zephyr/kernel.h>

#include "watchdog_timer/watchdog_timer.h"
#include "flash_system/flash_system.h"
#include "measurements/measurements_fsm_timer.h"
#include <zephyr/logging/log.h>
#include "wifi_config/wifi_apis.h"
LOG_MODULE_REGISTER(main_m);

int main(void)
{
    // Init watchdog
    init_watchdog();
    // Init flash system
    flash_system_init();

    // Start the measurements fsm
    init_measurements_fsm_timer();
    start_measurements_fsm_timer();

    // Init wifi and trigger wifi connection
    wifi_apis_wifi_init();
    wifi_apis_connect();

    return 0;
}
