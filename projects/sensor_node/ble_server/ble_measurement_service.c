// --- includes ----------------------------------------------------------------
#include "ble_measurement_service.h"
#include "ble_conn_control.h"
#include "../bme_280/temperature_humidity_sensor.h"
#include "../../common/common.h"
#include "../gpio/gpioif.h"
#include "../soil_moisture/soil_moisture.h"
#include "../light_sensor/light_sensor.h"
#include <logging/log.h>

#include <bluetooth/addr.h>

// --- logging settings --------------------------------------------------------
LOG_MODULE_REGISTER(ble_m);

// --- static functions declarations -------------------------------------------
static void on_cccd_changed_measurement(const struct bt_gatt_attr *attr, uint16_t value);
static ssize_t on_read_temperature(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                   void *buf, uint16_t len, uint16_t offset);
static ssize_t on_read_humidity(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                void *buf, uint16_t len, uint16_t offset);

// --- static functions definitions --------------------------------------------
/*
 * This function is called whenever the CCCD register has been changed by the
 * client
 *
 * TODO: not useful right now but might need in the future
 */
static void on_cccd_changed_measurement(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    switch (value)
    {
    case BT_GATT_CCC_NOTIFY:
        // Start sending stuff!
        break;

    case BT_GATT_CCC_INDICATE:
        // Start sending stuff via indications
        break;

    case 0:
        // Stop sending stuff
        break;

    default:
        LOG_INF("Error, CCCD has been set to an invalid value");
    }
}

static ssize_t on_read_temperature(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                   void *buf, uint16_t len, uint16_t offset)
{
    int32_t temperature_value;
#ifndef SW_SENSOR_EMULATION_MODE
    struct sensor_value temperature;

    // Take bme280 measurement
    measure_temperature();
    // Fetch the measurement on a local variable
    temperature = get_temperature_measurement();

    // Convert temperature from sensor_value format to int32_t
    temperature_value = temperature.val1 * 100 + temperature.val2 / 10000;
#else
    temperature_value = 3266;
#endif
    // Send the measurement
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &temperature_value,
                             sizeof(temperature_value));
}

static ssize_t on_read_humidity(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                void *buf, uint16_t len, uint16_t offset)
{

    int32_t humidity_value;

#ifndef SW_SENSOR_EMULATION_MODE
    struct sensor_value humidity;

    // Take bme280 measurement
    measure_humidity();
    // Fetch the measurement on a local variable
    humidity = get_humidity_measurement();

    // Convert humidity from sensor_value format to int32_t
    humidity_value = humidity.val1 * 100 + humidity.val2 / 10000;

#else 
    humidity_value = 6642;
#endif
    // Send the measurement
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &humidity_value,
                             sizeof(humidity_value));
}

static ssize_t on_read_light_exposure(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                      void *buf, uint16_t len, uint16_t offset)
{
    int32_t light_exposure;
#ifndef SW_SENSOR_EMULATION_MODE
    const struct device *bh1750_device = get_bh1750_device();
    struct sensor_value lux;
    sensor_sample_fetch_chan(bh1750_device, SENSOR_CHAN_LIGHT);
    sensor_channel_get(bh1750_device, SENSOR_CHAN_LIGHT, &lux);
    light_exposure = lux.val1;
#else
    light_exposure = 18000;
#endif
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &light_exposure,
                             sizeof(light_exposure));
}

static ssize_t on_read_soil_moisture(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                     void *buf, uint16_t len, uint16_t offset)
{
    int32_t soil_moisture;
#ifndef SW_SENSOR_EMULATION_MODE
    gpio_enable_soil_moisture();
    k_sleep(K_MSEC(50));
   
    adc_measure_soil_moisture();
    soil_moisture = adc_get_soil_moisture();
    gpio_disable_soil_moisture();
#else
    soil_moisture = 5000; // 50%
#endif
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &soil_moisture,
                             sizeof(soil_moisture));
}

BT_GATT_SERVICE_DEFINE(measurement_service, BT_GATT_PRIMARY_SERVICE(BT_UUID_MEASUREMENT_SERVICE),
                       BT_GATT_CHARACTERISTIC(BT_UUID_HUMIDITY, BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_READ, BT_GATT_PERM_READ, on_read_humidity, NULL, NULL),
                       BT_GATT_CCC(on_cccd_changed_measurement, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
                       BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE, BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_READ, BT_GATT_PERM_READ, on_read_temperature, NULL, NULL),
                       BT_GATT_CCC(on_cccd_changed_measurement, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
                       BT_GATT_CHARACTERISTIC(BT_UUID_SOIL_MOISTURE, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, on_read_soil_moisture, NULL, NULL),
                       BT_GATT_CCC(on_cccd_changed_measurement, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
                       BT_GATT_CHARACTERISTIC(BT_UUID_LIGHT_EXPOSURE, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, on_read_light_exposure, NULL, NULL),
                       BT_GATT_CCC(on_cccd_changed_measurement, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE));

// --- functions definitions ---------------------------------------------------
void measurement_ble_send(void *data, uint16_t len,
                          const struct bt_uuid *char_uuid, uint8_t attr_index)
{
    const struct bt_gatt_attr *attr = &measurement_service.attrs[attr_index];

    struct bt_gatt_notify_params params =
        {
            .uuid = char_uuid,
            .attr = attr,
            .data = data,
            .len = len,
        };

    // Check whether notifications are enabled or not
    // TODO: Define error codes and assign them to functions like this
    // to know which error occured: Maybe log them on an error handler
    // currently using printk
    if (bt_gatt_is_subscribed(get_ble_connection(), attr, BT_GATT_CCC_NOTIFY))
    {
        // Send the notification
        if (bt_gatt_notify_cb(get_ble_connection(), &params))
        {
            LOG_INF("Error, unable to send notification\n");
        }
    }
    else
    {
        LOG_INF("Warning, notification not enabled on the selected attribute\n");
    }
}
