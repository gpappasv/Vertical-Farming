#ifndef GPIO_INTERFACE_H
#define GPIO_INTERFACE_H

// --- includes ----------------------------------------------------------------
#include "../../common/com_protocol/com_protocol.h"

// --- functions declarations --------------------------------------------------
void gpio_row_control_init(void);
void gpio_row_control_msg_parse(message_control_gpios_t* p_gpios_control);

#endif // GPIO_INTERFACE_H
