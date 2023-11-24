// --- includes ----------------------------------------------------------------
#include <drivers/gpio.h>
#include <logging/log.h>
#include <zephyr.h>
#include "../../common/com_protocol/com_protocol.h"

// --- defines -----------------------------------------------------------------
#define ROW_1_WATER_GPIO 31
#define ROW_2_WATER_GPIO 30
#define ROW_3_WATER_GPIO 20
#define ROW_4_WATER_GPIO 13
#define ROW_5_WATER_GPIO 12

#define ROW_1_LIGHT_GPIO 11
#define ROW_2_LIGHT_GPIO 10
#define ROW_3_LIGHT_GPIO 9
#define ROW_4_LIGHT_GPIO 8
#define ROW_5_LIGHT_GPIO 7

#define ROW_1_FAN_GPIO 6
#define ROW_2_FAN_GPIO 5
#define ROW_3_FAN_GPIO 4
#define ROW_4_FAN_GPIO 3
#define ROW_5_FAN_GPIO 2

// --- static variables definitions --------------------------------------------
static const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));

// --- logging -----------------------------------------------------------------
LOG_MODULE_REGISTER(gpio_interface_m);

// --- functions definitions ---------------------------------------------------
void gpio_row_control_init(void)
{
    // configure water pins
    gpio_pin_configure(gpio_dev, ROW_1_WATER_GPIO, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, ROW_2_WATER_GPIO, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, ROW_3_WATER_GPIO, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, ROW_4_WATER_GPIO, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, ROW_5_WATER_GPIO, GPIO_OUTPUT);

    // configure light pins
    gpio_pin_configure(gpio_dev, ROW_1_LIGHT_GPIO, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, ROW_2_LIGHT_GPIO, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, ROW_3_LIGHT_GPIO, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, ROW_4_LIGHT_GPIO, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, ROW_5_LIGHT_GPIO, GPIO_OUTPUT);

    // configure fan pins
    gpio_pin_configure(gpio_dev, ROW_1_FAN_GPIO, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, ROW_2_FAN_GPIO, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, ROW_3_FAN_GPIO, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, ROW_4_FAN_GPIO, GPIO_OUTPUT);
    gpio_pin_configure(gpio_dev, ROW_5_FAN_GPIO, GPIO_OUTPUT);
}

void gpio_row_control_msg_parse(message_control_gpios_t* p_gpios_control)
{
    // LOG_INF("Controlling 9160 gpios");
    // Control fan for every row
    gpio_pin_set(gpio_dev, ROW_1_FAN_GPIO, p_gpios_control->row_fan_control[0]);
    gpio_pin_set(gpio_dev, ROW_2_FAN_GPIO, p_gpios_control->row_fan_control[1]);
    gpio_pin_set(gpio_dev, ROW_3_FAN_GPIO, p_gpios_control->row_fan_control[2]);
    gpio_pin_set(gpio_dev, ROW_4_FAN_GPIO, p_gpios_control->row_fan_control[3]);
    gpio_pin_set(gpio_dev, ROW_5_FAN_GPIO, p_gpios_control->row_fan_control[4]);

    // control water for every row
    gpio_pin_set(gpio_dev, ROW_1_WATER_GPIO, p_gpios_control->row_water_control[0]);
    gpio_pin_set(gpio_dev, ROW_2_WATER_GPIO, p_gpios_control->row_water_control[1]);
    gpio_pin_set(gpio_dev, ROW_3_WATER_GPIO, p_gpios_control->row_water_control[2]);
    gpio_pin_set(gpio_dev, ROW_4_WATER_GPIO, p_gpios_control->row_water_control[3]);
    gpio_pin_set(gpio_dev, ROW_5_WATER_GPIO, p_gpios_control->row_water_control[4]);

    // control light for every row
    gpio_pin_set(gpio_dev, ROW_1_LIGHT_GPIO, p_gpios_control->row_lights_control[0]);
    gpio_pin_set(gpio_dev, ROW_2_LIGHT_GPIO, p_gpios_control->row_lights_control[1]);
    gpio_pin_set(gpio_dev, ROW_3_LIGHT_GPIO, p_gpios_control->row_lights_control[2]);
    gpio_pin_set(gpio_dev, ROW_4_LIGHT_GPIO, p_gpios_control->row_lights_control[3]);
    gpio_pin_set(gpio_dev, ROW_5_LIGHT_GPIO, p_gpios_control->row_lights_control[4]);
}