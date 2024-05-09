// --- includes ----------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>

// --- defines -----------------------------------------------------------------

// --- enums -------------------------------------------------------------------
typedef enum
{
    COAP_FSM_WAIT_FOR_CONNECTION = 0x001,
    COAP_FSM_ROW_DATA_TO_SERVER_EVT = 0x002,
}coap_fsm_evts;

// --- function declarations ---------------------------------------------------
void coap_fsm_register_evt(coap_fsm_evts evt);