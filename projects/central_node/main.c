// --- includes ----------------------------------------------------------------
#include "internal_uart/internal_uart.h"
#include "watchdog_timer/watchdog_timer.h"
#include "flash_system/flash_system.h"
#include "measurements/measurements_fsm_timer.h"

// --- functions definitions ---------------------------------------------------
void main(void)
{
    // Init watchdog init
    init_watchdog();
    // Initialize internal uart (to communicate with 9160)
    internal_uart_init();
    flash_system_init();
    init_measurements_fsm_timer();
    start_measurements_fsm_timer();
}