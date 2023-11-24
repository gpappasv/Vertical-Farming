// --- includes ----------------------------------------------------------------
#include "environment_control_fsm.h"
#include "environment_control_config.h"
#include "sys/crc.h"
#include "../../common/com_protocol/com_protocol.h"
#include "../../common/common.h"
#include "../internal_uart/internal_uart.h"
#include <smf.h>
#include <logging/log.h>
#include <zephyr.h>

// --- defines -----------------------------------------------------------------
// 10% offset of the user requested soil moisture threshold
#define SOIL_MOISTURE_OFFSET 0.1
#define ONE_HUNDRED_PERCENT 100
// --- logging settings --------------------------------------------------------
LOG_MODULE_REGISTER(environment_control_m);

// --- enums -------------------------------------------------------------------
// List of states
enum ble_state_e
{
    ENV_CONTROL_INIT,
    ENV_CONTROL,
    ENV_CONTROL_WFE
};

// --- structs -----------------------------------------------------------------
// User defined object
struct user_object_s
{
    // This must be first
    struct smf_ctx ctx;
} env_control_fsm_user_object;

// --- static function declarations --------------------------------------------
static void env_control_init_run(void *o);

static void env_control_run(void *o);

static void env_control_wfe_run(void *o);

// --- static variables definitions --------------------------------------------
// Populate state table
static const struct smf_state env_control_states[] = {
    [ENV_CONTROL_INIT] = SMF_CREATE_STATE(NULL, env_control_init_run, NULL),
    [ENV_CONTROL] = SMF_CREATE_STATE(NULL, env_control_run, NULL),
    [ENV_CONTROL_WFE] = SMF_CREATE_STATE(NULL, env_control_wfe_run, NULL),
};

// --- variables definitions ---------------------------------------------------
struct k_event env_control_event;

// --- static function definitions ---------------------------------------------
// --- ENV_CONTROL_INIT state ---
static void env_control_init_run(void *o)
{
    k_event_init(&env_control_event);
    // load default if nothing stored
    // TODO: first check if there is a configuration stored in flash
    initialize_row_control_configuration();
    smf_set_state(SMF_CTX(&env_control_fsm_user_object), &env_control_states[ENV_CONTROL_WFE]);
}

// --- ENV_CONTROL state ---
static void env_control_run(void *o)
{
    // Construct the message to be sent to 9160 in order to control GPIOs
    message_control_gpios_t message_control_gpios = {0};
    message_control_gpios.type = MESSAGE_CONTROL_GPIOS;
    message_control_gpios.len = sizeof(message_control_gpios_t);
    // Control the fans/lights/water row by row
    // Check if automatic control is enabled -> if enabled, check the threshold
    // and control fan/light/water
    // If automatic control is disabled, blindly do what user has set for fan/light/water
    for (uint8_t row_index = 0; row_index < MAX_CONFIGURATION_ID; row_index++)
    {
        // Check if row is registered, if not, turn fan/light/water off
        if (get_row_registered(row_index))
        {
            // Check if manual control for the corresponding row is enabled
            if (!get_row_automatic_control(row_index))
            {
                // Control the fan
                if (get_row_fan_switch(row_index))
                {
                    message_control_gpios.row_fan_control[row_index] = true;
                    // Set fan on for row index
                    LOG_INF("Manual fan on for %d", row_index + 1);
                }
                else
                {
                    message_control_gpios.row_fan_control[row_index] = false;
                    // Set fan off for row index
                    LOG_INF("Manual fan off for %d", row_index + 1);
                }

                // Control the water
                if (get_row_water_switch(row_index))
                {
                    message_control_gpios.row_water_control[row_index] = true;
                    // Set water on for row index
                    LOG_INF("Manual water on for %d", row_index + 1);
                }
                else
                {
                    message_control_gpios.row_water_control[row_index] = false;
                    // Set water off for row index
                    LOG_INF("Manual water off for %d", row_index + 1);
                }

                // Control the lights
                if (get_row_light_switch(row_index))
                {
                    message_control_gpios.row_lights_control[row_index] = true;
                    // Set lights on for row index
                    LOG_INF("Manual lights on for %d", row_index + 1);
                }
                else
                {
                    message_control_gpios.row_lights_control[row_index] = false;
                    // Set lights off for row index
                    LOG_INF("Manual lights off for %d", row_index + 1);
                }
            }
            else // if automatic control
            {
                // Control the fan
                if ((get_row_current_temperature(row_index) > get_row_temp_threshold(row_index)) || (get_row_current_humidity(row_index) > get_row_hum_threshold(row_index)))
                {
                    message_control_gpios.row_fan_control[row_index] = true;
                    // Set fan on for row index
                    LOG_INF("Auto fan on for %d", row_index + 1);
                }
                else
                {
                    message_control_gpios.row_fan_control[row_index] = false;
                    // Set fan off for row index
                    LOG_INF("Auto fan off for %d", row_index + 1);
                }

                // Control the water
                if (get_row_current_soil_moisture(row_index) < get_row_soil_moisture_threshold(row_index))
                {
                    message_control_gpios.row_water_control[row_index] = true;
                    // Set water on for row index
                    LOG_INF("Auto water on for %d", row_index + 1);
                }
                // Water will be turned off after we exceed the threshold 10% of the user requested limit
                else if (get_row_current_soil_moisture(row_index) > get_row_soil_moisture_threshold(row_index) + SOIL_MOISTURE_OFFSET * get_row_soil_moisture_threshold(row_index))
                {
                    message_control_gpios.row_water_control[row_index] = false;
                    // Set water off for row index
                    LOG_INF("Auto water off for %d", row_index + 1);
                }
                // if soil moisture reaches 100%, water must be turned off TODO: Check if 100 should be lower, like 90%
                else if (get_row_current_soil_moisture(row_index) >= ONE_HUNDRED_PERCENT)
                {
                    message_control_gpios.row_water_control[row_index] = false;
                    // Set water off for row index
                    LOG_INF("Auto water off for %d", row_index + 1);
                }
                // Lights will be on as long as the row is registered
                message_control_gpios.row_lights_control[row_index] = true;
                    // Set lights on for row index
                LOG_INF("Auto lights on for %d", row_index + 1);
            }
        }
        else
        {
            message_control_gpios.row_fan_control[row_index] = false;
            message_control_gpios.row_water_control[row_index] = false;
            message_control_gpios.row_lights_control[row_index] = false;
            // Set fan/water/light off
            LOG_INF("Row not registered, so all off %d", row_index + 1);
        }
    }
    message_control_gpios.message_crc = crc16_ansi((uint8_t*)&message_control_gpios, sizeof(message_control_gpios_t) - sizeof(message_control_gpios.message_crc));

    // Send message to 9160 in order to control the gpios
    internal_uart_send_data((uint8_t*)&message_control_gpios, sizeof(message_control_gpios_t));
    smf_set_state(SMF_CTX(&env_control_fsm_user_object), &env_control_states[ENV_CONTROL_WFE]);
}

// --- ENV_CONTROL_WFE state ---
static void env_control_wfe_run(void *o)
{
    uint32_t events;
    events = k_event_wait(&env_control_event, ENV_CONTROL_MEASUREMENTS_TAKEN_EVT | ENV_CONTROL_USER_REQUEST_EVENT, true, K_FOREVER);
    smf_set_state(SMF_CTX(&env_control_fsm_user_object), &env_control_states[ENV_CONTROL]);
}

void env_control_fsm(void)
{
    int32_t ret;

    // Set initial state
    smf_set_initial(SMF_CTX(&env_control_fsm_user_object), &env_control_states[ENV_CONTROL_INIT]);

    // Run the state machine
    while (1)
    {
        // State machine terminates if a non-zero value is returned
        ret = smf_run_state(SMF_CTX(&env_control_fsm_user_object));
        if (ret)
        {
            // handle return code and terminate state machine
            break;
        }
    }
}

K_THREAD_DEFINE(env_control_fsm_id, ENV_CONTROL_STACKSIZE, env_control_fsm, NULL, NULL, NULL,
                ENV_CONTROL_PRIORITY, 0, 0);