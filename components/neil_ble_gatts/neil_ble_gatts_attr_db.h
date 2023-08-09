/// neil_ble_gatts_attr_db.h
///
/// @author     Nicholas H.R. Sims
///
/// @brief      GATT Attribute Table API Spec.

#ifndef neil_ble_gatts_attr_db_H_
#define neil_ble_gatts_attr_db_H_

#include <stdint.h>

#include "esp_gatt_defs.h"

#include "neil_ble_gatts_cfg.h"

// -------------------------------------------------------------
// Domain Structures
// -------------------------------------------------------------

/**
 * @brief       GATT Attribute Table. Used to configure a GATT server instance.
 */
typedef struct neil_ble_gatts_attr_db_s {
    uint16_t len;
    esp_gatts_attr_db_t *data;
} neil_ble_gatts_attr_db_t;

// -------------------------------------------------------------
// Procedures
// -------------------------------------------------------------

/**
 * @brief       Create a new GATT attribute table.
 */
neil_ble_gatts_attr_db_t *neil_ble_gatts_attr_db_init(const neil_ble_gatts_cfg_dev_t *dev_cfg);

/**
 * @brief       Tear down contents of a GATT Attribute Table.
 *
 * Warning: destructive process, do not use the input structure
 *          after tear down process without re-initializing.
 */
void neil_ble_gatts_attr_db_deinit(neil_ble_gatts_attr_db_t *attr_tab);

#endif // neil_ble_gatts_attr_db_H_
