/// vib_ble_gatt_table.h
///
/// @author     Nicholas H.R. Sims
///
/// @brief      GATT Attribute Table API Spec.

#ifndef VIB_BLE_GATT_TABLE_H_
#define VIB_BLE_GATT_TABLE_H_

#include <stdint.h>

#include "vib_ble_cfg.h"

// -------------------------------------------------------------
// Opaque Domain Structures
// -------------------------------------------------------------

/**
 * @brief       GATT Attribute Table. Used to configure a GATT server instance.
 */
typedef struct vib_ble_gatt_table_s vib_ble_gatt_table_t;

// -------------------------------------------------------------
// Procedures
// -------------------------------------------------------------

/**
 * @brief       Create a new GATT attribute table.
 */
vib_ble_gatt_table_t *vib_ble_gatt_table_init(const vib_ble_cfg_dev_t *dev_cfg);

/**
 * @brief       Tear down contents of a GATT Attribute Table.
 *
 * Warning: destructive process, do not use the input structure
 *          after tear down process without re-initializing.
 */
void vib_ble_gatt_table_deinit(vib_ble_gatt_table_t *attr_tab);

#endif // VIB_BLE_GATT_TABLE_H_
