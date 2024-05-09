#ifndef BLE_MEASUREMENT_SERVICE
#define BLE_MEASUREMENT_SERVICE

// --- includes ----------------------------------------------------------------
#include <stdint.h>

#include <zephyr/drivers/sensor.h>

#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

// --- functions declarations --------------------------------------------------
void measurement_ble_send(void *data, uint16_t len, 
                          const struct bt_uuid *char_uuid, uint8_t attr_index);

#endif // BLE_MEASUREMENT_SERVICE