#ifndef MEASUREMENT_FSM_H
#define MEASUREMENT_FSM_H

// --- defines -----------------------------------------------------------------
// size of stack area used by each thread
#define STACKSIZE 1024
// scheduling priority used by each thread
// TODO: probably add a central headerfile with priorities
#define PRIORITY 7

// --- enums -------------------------------------------------------------------
// List of states
enum measurements_fsm_evt_e
{
    MEASUREMENTS_FSM_RUN_EVT = 0xAA,
};

// --- extern variables declarations -------------------------------------------
extern struct k_sem read_response_sem;
extern struct k_sem at_least_one_active_connection_sem;

#endif // MEASUREMENT_FSM_H