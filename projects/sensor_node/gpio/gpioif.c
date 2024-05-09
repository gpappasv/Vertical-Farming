// --- includes ----------------------------------------------------------------
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

// --- defines -----------------------------------------------------------------
#define SOIL_MOISTURE_GPIO_ENABLE_PIN 13
#define GPIO1 DT_NODELABEL(gpio1)
// --- logging -----------------------------------------------------------------
LOG_MODULE_REGISTER(gpio_interface_m);

// --- static variables definitions --------------------------------------------

// --- static function definitions -------------------------------------------

static const struct device *get_gpio_dev(void)
{
    return DEVICE_DT_GET(GPIO1);
}

// --- function definitions -------------------------------------------
/**
 * @brief 
 * Initiaize gpio configuration.
 * E.g. output pins, drive strength etc.
 * This function contains the configuration
 * for all gpio needed.
 * 
 */
void gpio_interface_init(void)
{
    const struct device *gpio_dev = get_gpio_dev();
    if(gpio_dev == NULL)
    {
        LOG_ERR("GPIO dev is null");
        return;
    }
    /* Soil Moisture GPIO Enable --> Pin P01.8 */
    gpio_pin_configure(gpio_dev, SOIL_MOISTURE_GPIO_ENABLE_PIN, GPIO_OUTPUT);
    /* GPIO High drive mode */
    NRF_P1->PIN_CNF[8] |= (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos);

}

/**
 * @brief 
 * Enable soil moisture sensor
 * 
 */
void gpio_enable_soil_moisture(void)
{
    const struct device *gpio_dev = get_gpio_dev();
    if(gpio_dev == NULL)
    {
        LOG_ERR("GPIO dev is null");
        return;
    }
    gpio_pin_set(gpio_dev, SOIL_MOISTURE_GPIO_ENABLE_PIN, GPIO_OUT_PIN8_High);
}

/**
 * @brief 
 * Disable soil moisture sensor
 * 
 */
void gpio_disable_soil_moisture(void)
{
    const struct device *gpio_dev = get_gpio_dev();
    if(gpio_dev == NULL)
    {
        LOG_ERR("GPIO dev is null");
        return;
    }
    gpio_pin_set(gpio_dev, SOIL_MOISTURE_GPIO_ENABLE_PIN, GPIO_OUT_PIN8_Low);
}

