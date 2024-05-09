// --- includes ----------------------------------------------------------------
#include <zephyr/kernel.h>
#include "../internal_uart/internal_uart.h"
#include "../gpioif/gpio_interface.h"
#include <nrf_modem_at.h>
#include <zephyr/logging/log.h>
#include <modem/lte_lc.h>

// --- logging settings --------------------------------------------------------
LOG_MODULE_REGISTER(main_m);

// --- functions definitions ---------------------------------------------------
static void modem_connect(void)
{
	int ret;

	do {

		LOG_INF("Connecting to network.");
		LOG_INF("This may take several minutes.");

		ret = lte_lc_connect();
		if (ret < 0) {
			LOG_WRN("Failed to establish LTE connection (%d).", ret);
			LOG_WRN("Will retry in a minute.");
			lte_lc_offline();
			k_sleep(K_SECONDS(60));
		}
	} while (ret < 0);
}

int main(void)
{
    modem_connect();
    internal_uart_init();
    gpio_row_control_init();
    return 0;
}
