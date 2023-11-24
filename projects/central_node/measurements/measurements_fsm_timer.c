// --- includes ----------------------------------------------------------------
#include "measurements_fsm_timer.h"
#include "../measurements/measurements_fsm.h"
#include "zephyr.h"
// --- defines -----------------------------------------------------------------
// TODO: Probably set it from html page
#define MEASUREMENT_PERIOD_IN_SEC 20

// --- static variables definitions --------------------------------------------
static struct k_timer measurements_fsm_timer;
extern struct k_event measurements_fsm_event;

// --- interrupt handlers  -------------------------------------------
static void measurements_fsm_timer_handler(struct k_timer *timer_id)
{
    k_event_post(&measurements_fsm_event, MEASUREMENTS_FSM_RUN_EVT);
}

// --- functions declarations -------------------------------------------
void init_measurements_fsm_timer(void)
{
    k_timer_init(&measurements_fsm_timer, measurements_fsm_timer_handler, NULL);
}

void start_measurements_fsm_timer(void)
{
    k_timer_start(&measurements_fsm_timer, K_SECONDS(0), K_SECONDS(MEASUREMENT_PERIOD_IN_SEC));
}
