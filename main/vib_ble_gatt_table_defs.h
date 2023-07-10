#ifndef VIB_BLE_GATT_TABLE_DEFS_H_
#define VIB_BLE_GATT_TABLE_DEFS_H_

#include <stdint.h>

#include "esp_gatt_defs.h"

// -------------------------------------------------------------
// Data Structures
// -------------------------------------------------------------

typedef struct vib_ble_gatt_table_s {
    uint16_t len;
    esp_gatts_attr_db_t *data;
} vib_ble_gatt_table_t;

#endif // VIB_BLE_GATT_TABLE_DEFS_H_
