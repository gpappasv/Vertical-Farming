#ifndef TEMPERATURE_HUMIDITY_SENSOR_H
#define TEMPERATURE_HUMIDITY_SENSOR_H

// --- includes ----------------------------------------------------------------
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

// --- defines -----------------------------------------------------------------
// Tested devices: bosch_bme280, aosong_dht
#ifdef BME_280
    #define TEMP_HUM_SENSOR_TYPE bosch_bme280
#else
    #define TEMP_HUM_SENSOR_TYPE aosong_dht
#endif

// --- structs -----------------------------------------------------------------
typedef struct bme_measurements_s
{
    // Temp 21.62 oC is represented as val1 = 21, val2 = 620000
    struct sensor_value temperature;
    // Humidity 44.62% is represented as val1 = 44, val2 = 620000
    struct sensor_value humidity;
} bme_measurements_t;

// --- functions declarations --------------------------------------------------
void measure_temperature(void);
void measure_humidity(void);
bool init_temp_hum_sensor(void);
struct sensor_value get_temperature_measurement(void);
struct sensor_value get_humidity_measurement(void);

#endif // TEMPERATURE_HUMIDITY_SENSOR_H
