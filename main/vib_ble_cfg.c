/// vib_ble_cfg.c
///
/// @author     Nicholas H.R. Sims
///
/// @brief      Characteristic Configuration Map API.

#include <stdint.h>
#include <stdlib.h>

#include "vib_ble_cfg.h"

// -------------------------------------------------------------
// Configuration Map
// -------------------------------------------------------------

typedef struct vib_ble_cfg_chr_map_s {
    size_t len;
    uint16_t offset;
    uint8_t *flags;
    vib_ble_cfg_chr_t **data;
} vib_ble_cfg_chr_map_t;

// --- Allocate and configure a new handle-to-characteristic-config map.
vib_ble_cfg_chr_map_t *
vib_ble_cfg_chr_map_init(const vib_ble_cfg_dev_t *dev_cfg,
                         const uint16_t offset) {

    vib_ble_cfg_chr_map_t *map = malloc(sizeof(vib_ble_cfg_chr_map_t));

    uint8_t length = vib_ble_cfg_handle_range(dev_cfg);

    map->len    = length;
    map->offset = offset;

    map->flags = malloc(length * sizeof(uint8_t));

    // Initialize the internal mapping table with
    // length * the size of a characteristic config pointer.
    map->data = malloc(length * sizeof(vib_ble_cfg_chr_t *));

    return map;
}

// HACK: No Error Check
void vib_ble_cfg_chr_map_set(vib_ble_cfg_chr_map_t *map, uint16_t handle,
                             vib_ble_cfg_chr_t *value) {
    *(map->data + (handle - map->offset)) = value;
}

// HACK: No Error Check
vib_ble_cfg_chr_t *vib_ble_cfg_chr_map_get(vib_ble_cfg_chr_map_t *map,
                                           uint16_t key) {
    return *(map->data + (key - map->offset));
}

// HACK: Error Checking
void vib_ble_cfg_chr_map_deinit(vib_ble_cfg_chr_map_t *map) {

    // Set all values in data array to null.
    size_t length = map->len;
    for (typeof(map->data) spool; spool < spool + length; spool++) {
        spool = NULL;
    }

    free(map->data);
    free(map);
}

// -------------------------------------------------------------
// Auxiliary Procedures
// -------------------------------------------------------------

/**
 * @brief       Get the length of memory required for a GATT table
 *              based on a device configuration structure.
 *
 *              This length is also used to hold attribute handles,
 *              and can be used to determine how many 2-byte memory cells are
 *              needed to create a handle-to-configuration-entry map.
 */
uint8_t vib_ble_util_handle_range(const vib_ble_cfg_dev_t *const dev_cfg) {

    static const uint8_t handle_size = 2;

    // One handle per service.
    uint8_t length = dev_cfg->svc_tab_len;

    for (uint8_t index = 0; index < dev_cfg->svc_tab_len; index++) {
        // Two additional handles per characteristic.
        // HACK: This will expand to 3 in the future to support
        //       client characteristic configuration parameters
        //       (requiring an additional handle)
        length += handle_size * (*(dev_cfg->svc_tab + index)).chr_tab_len;
    }

    return length;
}
