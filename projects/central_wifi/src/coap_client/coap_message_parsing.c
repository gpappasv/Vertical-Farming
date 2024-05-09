// --- includes ----------------------------------------------------------------
#include "coap_message_parsing.h"

#include "com_protocol.h"
#include <zephyr/logging/log.h>
#include "environment_control/environment_control_fsm.h"
#include "environment_control/environment_control_config.h"

// --- logging settings --------------------------------------------------------
LOG_MODULE_DECLARE(coap_m);

// --- variables declarations --------------------------------------------------
extern struct k_event env_control_event;

// --- functions definitions ---------------------------------------------------
uint8_t process_coap_rx_message(const uint8_t *rx_buf)
{
    message_operation_result_t *p_msg_op_result;
    message_coap_row_control_user_data_t *p_msg_coap_row_control_user_data;
    message_coap_row_thresholds_user_data_t *p_msg_coap_row_thresholds_user_data;
    uint8_t ret = GENERIC_ERROR;

    switch (rx_buf[MSG_TYPE_POSITION])
    {
    // TODO: probably allow the user to set fan or water or light switch on/off 
    // separately without configuring all of them on every request
    // keep the same message and just add keep 2 fields: switch_id and on/off flag
    case MESSAGE_COAP_ROW_CONTROL_USER_DATA:
        p_msg_coap_row_control_user_data = (message_coap_row_control_user_data_t *)rx_buf;
        // Set the user data on the environment control module
        // NOTE: p_msg_coap_row_user_data->row_id - 1 is used as user will send the message with row ids from 1 to 5
        // but the following functions use this row id to index the row_control_config[]
        set_row_automatic_control(p_msg_coap_row_control_user_data->is_automatic_control, p_msg_coap_row_control_user_data->row_id - 1);
        set_row_fan_switch(p_msg_coap_row_control_user_data->is_fan_active, p_msg_coap_row_control_user_data->row_id - 1);
        set_row_light_switch(p_msg_coap_row_control_user_data->is_light_on, p_msg_coap_row_control_user_data->row_id - 1);
        set_row_water_switch(p_msg_coap_row_control_user_data->is_water_on, p_msg_coap_row_control_user_data->row_id - 1);

        // Notify environment control fsm that user sent a request
        k_event_post(&env_control_event, ENV_CONTROL_USER_REQUEST_EVENT);
        // Call function to submit a work item in order to update those parameters in flash
        update_row_control_config_params_in_nvs(p_msg_coap_row_control_user_data->row_id - 1);
        // TODO: Should reply with operation result message
        ret = SUCCESS;
        break;
    case MESSAGE_COAP_ROW_THRESHOLDS_USER_DATA:
        p_msg_coap_row_thresholds_user_data = (message_coap_row_thresholds_user_data_t *)rx_buf;
        // Only set the thresholds
        set_row_hum_threshold(p_msg_coap_row_thresholds_user_data->humidity_threshold, p_msg_coap_row_thresholds_user_data->row_id - 1);
        set_row_temp_threshold(p_msg_coap_row_thresholds_user_data->temp_threshold, p_msg_coap_row_thresholds_user_data->row_id - 1);
        set_row_soil_moisture_threshold(p_msg_coap_row_thresholds_user_data->soil_moisture_threshold, p_msg_coap_row_thresholds_user_data->row_id - 1);
        set_row_light_threshold(p_msg_coap_row_thresholds_user_data->light_threshold, p_msg_coap_row_thresholds_user_data->row_id - 1);

        // Notify environment control fsm that user sent a request
        k_event_post(&env_control_event, ENV_CONTROL_USER_REQUEST_EVENT);
        // Call function to submit a work item in order to update those parameters in flash
        update_row_control_config_params_in_nvs(p_msg_coap_row_thresholds_user_data->row_id - 1);
        ret = SUCCESS;
        break;
    case MESSAGE_OPERATION_RESULT:
        p_msg_op_result = (message_operation_result_t *)rx_buf;
        // TODO: Parse the message and check the error code
        ret = SUCCESS;
        break;
    default:
        break;
    }

    return ret;
}