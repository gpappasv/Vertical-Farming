// --- includes ----------------------------------------------------------------
#include "temperature_humidity_sensor.h"
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>

// --- logging settings --------------------------------------------------------
LOG_MODULE_REGISTER(temperature_humidity_sensor_m);

// --- static variables definitions --------------------------------------------
// Struct variable to store temperature and humidity measurements
static bme_measurements_t temp_hum_measurements = {0};

// --- functions definitions ---------------------------------------------------
/**
 * @brief Function to check if temp_hum_sensor device is ready for use
 * 
 * @return true if ready for use
 * @return false if not ready
 */
bool init_temp_hum_sensor(void)
{
    const static struct device *temp_hum_sensor_dev = DEVICE_DT_GET_ANY(TEMP_HUM_SENSOR_TYPE);
    if(!device_is_ready(temp_hum_sensor_dev))
    {
        LOG_INF("temp hum sensor device not ready during init");
        return false;
    }

    return true;
}

// Update temp_hum_measurements struct with latest measurements
// TODO: maybe add error code?
void measure_temperature(void)
{
    int err;

#ifdef CONFIG_BME280
    const static struct device *bme_spi_device = DEVICE_DT_GET_ANY(nordic_nrf_spim);
    pm_device_action_run(bme_spi_device, PM_DEVICE_ACTION_RESUME);
#endif

    const static struct device *temp_hum_sensor_dev = DEVICE_DT_GET_ANY(TEMP_HUM_SENSOR_TYPE);

    if ((temp_hum_sensor_dev == NULL) || !device_is_ready(temp_hum_sensor_dev))
    {
        // TODO: maybe add a define to enable and disable debug prints
        LOG_INF("Check temp_hum_sensor sensor: not available\n");
        return;
    }

    // Take measurements and store them on sensor buffer
    err = sensor_sample_fetch_chan(temp_hum_sensor_dev, SENSOR_CHAN_AMBIENT_TEMP);
    if (err != 0)
    {
        LOG_INF("Error in fetching sample %d", err);
    }

    // Update temperature value
    err = sensor_channel_get(temp_hum_sensor_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_hum_measurements.temperature);
    if (err != 0)
    {
        LOG_INF("Error in fetching sample %d", err);
    }
#ifdef CONFIG_BME280
    // Disable spi to save power
    pm_device_action_run(bme_spi_device, PM_DEVICE_ACTION_SUSPEND);
#endif
}


// Update temp_hum_measurements struct with latest measurements
// TODO: maybe add error code?
void measure_humidity(void)
{
    int err;

#ifdef CONFIG_BME280
    const static struct device *bme_spi_device = DEVICE_DT_GET_ANY(nordic_nrf_spim);
    pm_device_action_run(bme_spi_device, PM_DEVICE_ACTION_RESUME);
#endif

    const static struct device *temp_hum_sensor_dev = DEVICE_DT_GET_ANY(TEMP_HUM_SENSOR_TYPE);

    if ((temp_hum_sensor_dev == NULL) || (!device_is_ready(temp_hum_sensor_dev)))
    {
        // TODO: maybe add a define to enable and disable debug prints
        LOG_INF("Check temp_hum_sensor sensor: not available\n");
    }

    // Take measurements and store them on sensor buffer
    err = sensor_sample_fetch_chan(temp_hum_sensor_dev, SENSOR_CHAN_HUMIDITY);
    if (err != 0)
    {
        LOG_INF("Error in fetching sample %d", err);
    }

    // Update humidity value
    err = sensor_channel_get(temp_hum_sensor_dev, SENSOR_CHAN_HUMIDITY, &temp_hum_measurements.humidity);
    if (err != 0)
    {
        LOG_INF("Error in retreiving sample %d", err);
    }

#ifdef CONFIG_BME280
    // Disable spi to save power
    pm_device_action_run(bme_spi_device, PM_DEVICE_ACTION_SUSPEND);
#endif
}

// --- getters -----------------------------------------------------------------
// Getter for temperature value
struct sensor_value get_temperature_measurement(void)
{
    return temp_hum_measurements.temperature;
}

// Getter for humidity value
struct sensor_value get_humidity_measurement(void)
{
    return temp_hum_measurements.humidity;
}