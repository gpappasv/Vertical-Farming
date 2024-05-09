// --- includes ----------------------------------------------------------------
#include "coap_fsm.h"
#include "coap_client.h"
#include "../internal_uart/internal_uart.h"
#include <stdbool.h>
#include <zephyr/kernel.h>
#include "../inventory/inventory.h"
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
#include <zephyr/net/coap.h>
#include <zephyr/net/socket.h>
#include "coap_message_parsing.h"
#include <modem/lte_lc.h>
#include <nrf_modem_at.h>
#include "../timestamp_module/timestamp.h"

// --- logging settings --------------------------------------------------------
LOG_MODULE_REGISTER(coap_m);

// --- defines -----------------------------------------------------------------
// size of stack area used by each thread
#define COAP_OBS_STACKSIZE 2048
// scheduling priority used by each thread
#define COAP_FSM_PRIORITY 6
#define COAP_OBS_PRIORITY 6

// --- enums -------------------------------------------------------------------
// List of states
enum ble_state_e
{
    // Initialize coap client
    COAP_CLIENT_INIT,
    // Send mean row data to cloud
    COAP_CLIENT_SEND_MEAS,
    // Send device info to cloud (battery/mac)
    COAP_CLIENT_SEND_DEV_INFO,
    // FSM sleep and wait for event
    COAP_CLIENT_WAIT,
    // Reconnect state
    COAP_CLIENT_NBIOT_RECONNECT
};

// --- static function declarations --------------------------------------------
static void coap_observe_loop(void);
static void coap_client_init_run(void *o);

static void coap_client_send_meas_entry(void *o);
static void coap_client_send_meas_run(void *o);
static void coap_client_send_meas_exit(void *o);

static void coap_client_send_dev_info_entry(void *o);
static void coap_client_send_dev_info_run(void *o);
static void coap_client_send_dev_info_exit(void *o);

static void coap_client_wait_run(void *o);

static void coap_nbiot_reconnect_run(void *o);

static void lte_handler(const struct lte_lc_evt *const evt);

// --- extern variables declarations -------------------------------------------
extern struct k_event uart_communication_events;

// --- static variables definitions --------------------------------------------
// Populate state table
static const struct smf_state coap_client_states[] = {
    [COAP_CLIENT_INIT] = SMF_CREATE_STATE(NULL, coap_client_init_run, NULL),
    [COAP_CLIENT_SEND_MEAS] = SMF_CREATE_STATE(coap_client_send_meas_entry, coap_client_send_meas_run, coap_client_send_meas_exit),
    [COAP_CLIENT_SEND_DEV_INFO] = SMF_CREATE_STATE(coap_client_send_dev_info_entry, coap_client_send_dev_info_run, coap_client_send_dev_info_exit),
    [COAP_CLIENT_WAIT] = SMF_CREATE_STATE(NULL, coap_client_wait_run, NULL),
    [COAP_CLIENT_NBIOT_RECONNECT] = SMF_CREATE_STATE(NULL, coap_nbiot_reconnect_run, NULL),
};

// --- variables definitions ---------------------------------------------------
// User defined object
struct user_object_s
{
    // This must be first
    struct smf_ctx ctx;
    row_mean_data_t *row_mean_data_inventory;
    measurements_data_t *measurements_data_inventory;
} coap_fsm_user_object;

// --- static function definitions ---------------------------------------------
// --- State COAP_CLIENT_INIT
static void coap_client_init_run(void *o)
{
    lte_lc_register_handler(lte_handler);
    // Disable modem PSM
    lte_lc_psm_req(false);
    // Initialize and connect to coap server
    // Check if errors on coap client. If there are, then return
    if (coap_client_init() < 0)
    {
        k_sleep(K_SECONDS(5));
        return;
    }
    else
    {
        // Register observe on the userpayload resource
        // TODO: GPA: Uncomment the following 3 lines in order to observe again
        char obs_resource[] = "userpayload";
        coap_observe(obs_resource, strlen(obs_resource));
        initialize_observe_renew();
        // -----------------------------------------
        // Set next state -> go to wait state until send event comes
        smf_set_state(SMF_CTX(&coap_fsm_user_object), &coap_client_states[COAP_CLIENT_WAIT]);
    }
}

// --- State COAP_CLIENT_SEND_MEAS
static void coap_client_send_meas_entry(void *o)
{
    struct user_object_s *user_ctx = (struct user_object_s *)o;
    // Get latest row mean data inventory
    user_ctx->row_mean_data_inventory = get_row_mean_data_inventory();
}
static void coap_client_send_meas_run(void *o)
{
    // Debug variable
    static uint32_t log_counter = 0;

    struct user_object_s *user_ctx = (struct user_object_s *)o;
    // Define the coap resource to send the data
    char resource[] = "rowmeandata";

    message_coap_row_mean_data_t coap_msg_buffer = {0};

    // Go through the row mean data inventory and for every registered row
    // send the data to cloud. A row will be registered if 52840 sent data for it
    LOG_INF("SEND_TO_CLOUD --- %d", log_counter);
    for (int index = 0; index < MAX_CONFIGURATION_ID; index++)
    {
        // Check if row is registered
        if (user_ctx->row_mean_data_inventory[index].is_row_registered)
        {
            // Construct the coap message
            create_coap_row_mean_data_message(&user_ctx->row_mean_data_inventory[index], &coap_msg_buffer, get_timestamp());
            LOG_INF("row_num: %d; temperature: %d; idx: %d", user_ctx->row_mean_data_inventory[index].row_id,user_ctx->row_mean_data_inventory[index].mean_row_temp, log_counter);
            // Send the coap message to coap server
            coap_put((uint8_t *)resource, strlen(resource), (uint8_t *)&coap_msg_buffer, sizeof(message_coap_row_mean_data_t));
        }
    }
    log_counter++;
    // Set next state
    smf_set_state(SMF_CTX(&coap_fsm_user_object), &coap_client_states[COAP_CLIENT_SEND_DEV_INFO]);
}
static void coap_client_send_meas_exit(void *o)
{
    // After sending the data to coap cloud server, clear the inventories
    reset_row_mean_data_inventory();
}

// --- State COAP_CLIENT_SEND_DEV_INFO
static void coap_client_send_dev_info_entry(void *o)
{
    struct user_object_s *user_ctx = (struct user_object_s *)o;
    // Get latest row mean data inventory
    user_ctx->measurements_data_inventory = get_measurements_data_inventory();
}
static void coap_client_send_dev_info_run(void *o)
{
    // if(send)
    // send
    smf_set_state(SMF_CTX(&coap_fsm_user_object), &coap_client_states[COAP_CLIENT_WAIT]);
}
static void coap_client_send_dev_info_exit(void *o)
{
    // This should be cleared on the coap client send dev info state
    reset_measurements_inventory();
}

// --- State COAP_CLIENT_WAIT
static void coap_client_wait_run(void *o)
{
    uint32_t events;
    events = k_event_wait(&uart_communication_events, COAP_ROW_DATA_TO_SERVER_EVT | COAP_NBIOT_RECONNECT_EVT, true, K_FOREVER);
    if(events == COAP_NBIOT_RECONNECT_EVT)
    {
        smf_set_state(SMF_CTX(&coap_fsm_user_object), &coap_client_states[COAP_CLIENT_NBIOT_RECONNECT]);
    }
    else
    {
        smf_set_state(SMF_CTX(&coap_fsm_user_object), &coap_client_states[COAP_CLIENT_SEND_MEAS]);
    }
}

// --- State COAP_CLIENT_NBIOT_RECONNECT
static void coap_nbiot_reconnect_run(void *o)
{
    int err;
    err = lte_lc_connect();
    // LOG_INF("Error status in nbiot re-connection: %d", err);
    // TODO: Probably add system reset at this point, or retries until success
    smf_set_state(SMF_CTX(&coap_fsm_user_object), &coap_client_states[COAP_CLIENT_WAIT]);
}

/**
 * @brief Callback function that is triggered when an lte event happens
 *        in case of rrc connection event, an lte_reception_evt is posted
 *
 * @param evt
 */
static void lte_handler(const struct lte_lc_evt *const evt)
{
    switch (evt->type)
    {
    case LTE_LC_EVT_RRC_UPDATE:
        break;
    case LTE_LC_EVT_NW_REG_STATUS:
        if((evt->nw_reg_status == LTE_LC_NW_REG_NOT_REGISTERED)
            || (evt->nw_reg_status == LTE_LC_NW_REG_REGISTRATION_DENIED)
            || (evt->nw_reg_status == LTE_LC_NW_REG_UNKNOWN)
            || (evt->nw_reg_status == LTE_LC_NW_REG_UICC_FAIL))
        {
            // LOG_INF("NB-IoT Network Lost, need reconnect: %d", evt->nw_reg_status);
            k_event_post(&uart_communication_events, COAP_NBIOT_RECONNECT_EVT);
        }
        else if((evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME)
            || (evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_ROAMING))
        {
            // LOG_INF("Reconnected to NB-IoT Network! %d", evt->nw_reg_status);
        }
        break;
    default:
        break;
    }
}

/**
 * @brief COAP FSM
 *
 */
void coap_fsm(void)
{
    int32_t ret;
    // Set initial state
    smf_set_initial(SMF_CTX(&coap_fsm_user_object), &coap_client_states[COAP_CLIENT_INIT]);

    // Run the state machine
    while (1)
    {
        // State machine terminates if a non-zero value is returned
        ret = smf_run_state(SMF_CTX(&coap_fsm_user_object));
        if (ret)
        {
            // handle return code and terminate state machine
            // LOG_INF("COAP fsm terminated: %d", ret);
            break;
        }
    }
}

/**
 * @brief Loop that runs on a dedicated thread in order to observe a coap resource
 *        TODO: Probably write this function in a cleaner way
 *              The observe tokens should be inited in this function maybe?
 *
 */
static void coap_observe_loop(void)
{
    int err, received;

    uint8_t receive_buffer[APP_COAP_MAX_MSG_LEN];
    struct coap_packet reply;
    const uint8_t *payload = 0;
    uint16_t payload_len;
    uint8_t token[8];
    uint16_t token_len;


    while (1)
    {
        // This loop will run with period of 20ms until something is received from nbiot
        // after a packet is received wait for next nbiot event because of lte_reception_evt
        k_sleep(K_MSEC(100));
        // --- TODO: *********** Move this part to coap_client on a function
        // Bytes received
        received = recv(coap_get_socket(), receive_buffer, sizeof(receive_buffer), MSG_DONTWAIT);
        if (received < 0)
        {
            // // LOG_INF("Socket error, exit...");
            continue;
        }

        if (received == 0)
        {
            // LOG_INF("Empty datagram");
            continue;
        }

        // --- parse get response
        err = coap_packet_parse(&reply, receive_buffer, received, NULL, 0);
        if (err < 0)
        {
            // LOG_INF("Malformed response received: %d", err);
            continue;
        }

        // payload will contain the packet payload, and payload_len will know the
        // length of the payload
        payload = coap_packet_get_payload(&reply, &payload_len);
        if (payload == NULL)
        {
            // LOG_INF("Null coap payload received");
            continue;
        }
        token_len = coap_header_get_token(&reply, token);

        // get_obs_token() will return a pointer to a variable that is uint16_t
        if ((token_len != sizeof(uint16_t)) ||
            (memcmp(get_obs_token(), token, sizeof(uint16_t)) != 0))
        {
            // LOG_INF("Invalid token received: 0x%02x%02x",
            //        token[1], token[0]);
        }
        else
        {
            // LOG_INF("Valid token received");

            // Parse the message, TODO: use the returned error code
            process_coap_rx_message(payload);
        }
        // --- TODO: ***********
    }
}

K_THREAD_DEFINE(coap_fsm_id, STACKSIZE, coap_fsm, NULL, NULL, NULL,
                COAP_FSM_PRIORITY, 0, 0);

// Observe loop - Commenting out to save data?? TODO: Dont know if it actually saves data
K_THREAD_DEFINE(coap_observe_id, COAP_OBS_STACKSIZE, coap_observe_loop, NULL, NULL, NULL,
               COAP_OBS_PRIORITY, 0, 0);