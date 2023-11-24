#ifndef MEASUREMENTS_FSM_TIMER
#define MEASUREMENTS_FSM_TIMER

// --- includes ----------------------------------------------------------------
#include <stdint.h>

// --- defines -----------------------------------------------------------------
// TODO: Probably set it from html page
#define MEASUREMENT_PERIOD_IN_SEC 30
#define MEASUREMENTS_SEND_TO_CLOUD_PERIOD_IN_SEC 5400

// --- functions declarations --------------------------------------------------
void init_measurements_fsm_timer(void);
void start_measurements_fsm_timer(void);

#endif /* MEASUREMENTS_FSM_TIMER */
