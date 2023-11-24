// --- includes ----------------------------------------------------------------
#include <kernel.h>
#include <drivers/sensor.h>
#include <init.h>
#include <sys/byteorder.h>
#include <sys/__assert.h>
#include <logging/log.h>
#include <pm/device.h>
#include "light_sensor.h"

// --- defines -----------------------------------------------------------------
#define BH1750_MEASUREMENT_MODE BH1750_MODE_SINGLE
#define BH1750_MEASUREMENT_RES BH1750_LOW_RES

/*  24 ms is the max time for measurement in low resolution mode. 180 ms for high resolution modes. */
#define BH1750_MEASUREMENT_TIME ((BH1750_MEASUREMENT_RES == 0x2) ? 24 : 180)

// --- logging -----------------------------------------------------------------
LOG_MODULE_REGISTER(bh1750_drv_m);

// --- struct definitions -----------------------------------------------------
struct bh1750_device_t
{
    const struct device *i2c;
    uint16_t lux;
    uint8_t data_h_byte;
    uint8_t data_l_byte;
};

// --- static variables definitions --------------------------------------------
static struct bh1750_device_t bh1750;
static const struct device *bh1750_dev = DEVICE_DT_GET_ANY(rohm_bh1750);

// --- static function definitions -------------------------------------------

/**
 * @brief
 * Sensor API get function.
 * Provides the result of the latest stored measurement.
 * @param dev
 * @param chan
 * @param val
 * @return int
 */
static int bh1750_channel_get(const struct device *dev,
                              enum sensor_channel chan,
                              struct sensor_value *val)
{
    struct bh1750_device_t *bh1750_data = dev->data;
    val->val1 = bh1750_data->lux;
    val->val2 = 0;
    return 0;
}

/**
 * @brief
 * Sensor API fetch function.
 * Requests a measurement to be made according
 * to the requested measurement mode and resolution
 * and then reads back the result. This result is
 * stored in the internal static bh1750 struct.
 *
 * @param dev
 * @param chan
 * @return int
 */
static int bh1750_sample_fetch(const struct device *dev,
                               enum sensor_channel chan)
{
    struct bh1750_device_t *bh1750_data = dev->data;

    /* I2C Request Low Resolution Measurement */

    uint8_t tx_data = BH1750_MEASUREMENT_MODE | BH1750_MEASUREMENT_RES;
    uint8_t rx_data[2];

    pm_device_action_run(bh1750_data->i2c, PM_DEVICE_ACTION_RESUME);

    int err;
    err = i2c_write(bh1750_data->i2c, &tx_data, 1, BH1750_I2C_ADDRESS);

    k_sleep(K_MSEC(BH1750_MEASUREMENT_TIME));

    /* Read the result */
    i2c_read(bh1750_data->i2c, rx_data, 2, BH1750_I2C_ADDRESS);
    if (err < 0)
    {
        LOG_INF("Read from BH1750 failed");
        return err;
    }

    pm_device_action_run(bh1750_data->i2c, PM_DEVICE_ACTION_SUSPEND);

    bh1750_data->data_h_byte = rx_data[0];
    bh1750_data->data_l_byte = rx_data[1];

    /* LUX Calculation */

    uint16_t tmp = (rx_data[0] << 8) | rx_data[1];
    uint16_t lux = tmp / 1.2;

    bh1750_data->lux = lux;
    return 0;
}

/* Sensor driver API */
static const struct sensor_driver_api bh1750_driver_api = {
    .sample_fetch = bh1750_sample_fetch,
    .channel_get = bh1750_channel_get,
};

// --- function definitions -------------------------------------------

/**
 * @brief Get the bh1750 device object
 *
 * @return const struct device*
 */
const struct device *get_bh1750_device(void)
{

    return bh1750_dev;
}
/**
 * @brief
 * BH1750 Initialization function.
 * This function is called through zephyr in
 * the POST_KERNEL stage. It sets the device
 * in powerdown.
 *
 * @param dev
 * @return int
 */
int bh1750_init(const struct device *dev)
{
    struct bh1750_device_t *drv_data = dev->data;
    int16_t err;

    drv_data->i2c = device_get_binding(DT_INST_BUS_LABEL(0));

    if (drv_data->i2c == NULL)
    {
        LOG_INF("Could not get pointer to bh1750 bus %d", DT_INST_BUS_LABEL(0));
        return -EINVAL;
    }

    /* Set to powerdown */

    uint8_t data = 0x0;
    err = i2c_write(drv_data->i2c, &data, 1, BH1750_I2C_ADDRESS);

    if (err < 0)
    {
        LOG_INF("Set device power down failed");
        return err;
    }

    return 0;
}

DEVICE_DT_INST_DEFINE(0, bh1750_init, NULL, &bh1750,
                      NULL, POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,
                      &bh1750_driver_api);
