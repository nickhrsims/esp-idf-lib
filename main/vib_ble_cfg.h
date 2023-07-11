/// vib_ble_cfg.h
///
/// @author     Nicholas H.R. Sims
///
/// @brief      Bluetooth Low-Energy Domain API.

#ifndef VIB_BLE_CFG_H_
#define VIB_BLE_CFG_H_

#include "esp_bt_defs.h"

// -------------------------------------------------------------
// Freudensong UUID System
// -------------------------------------------------------------
//
// Base UUID Auxiliary Macro
//
// Wraps a 16-bit attribute UUID within a base UUID.
//
// Format:
//     XX - 8-bit Service Index
//     YY - 8-bit Characteristic Index
//     C2D5B9D6-XXYY-452E-84D1-0A0C537A36D7
//
//     XX:    SERVICE       @ 11
//     YY: CHARACTERISTIC   @ 10
//
// Parameters:
//     (uint8_t)    service_index     - Internal Service ID.
//
//     (uint8_t) characteristic_index - Internal Characteristic ID.
//                                NOTE: Must be `0` when declaring a service.
//
// Note:
//     Parameters (atow) are not used for internal purposes, any range of
//     128-bit UUIDs will suffice that do not conflict the with the Bluetooth
//     Low-Energy spec 128-bit UUID range.
//
//     i.e. 0000NNNN-0000-1000-8000-00805F9B34FB
//
// Usage:
//     VIB_BLE_UUID_128(service_index, characteristic_index)
//
// @see https://www.uuidgenerator.net/
#define VIB_BLE_UUID_128(XX, YY)                                               \
    {                                                                          \
        0xD7, 0x36, 0x7A, 0x53, 0x0C, 0x0A, 0xD1, 0x84, 0x2E, 0x45, YY, XX,    \
            0xD6, 0xB9, 0xD5, 0xC2                                             \
    }

/// Get service index (by-convention) from UUID.
#define VIB_BLE_UUID_128_GET_SVC_INDEX(uuid128) uuid128[11]

/// Get characteristic index (by-convention) from UUID.
#define VIB_BLE_UUID_128_GET_CHR_INDEX(uuid128) uuid128[10]

// -------------------------------------------------------------
// Configuration Flags
// -------------------------------------------------------------

/// If set on device config, will generate UUIDs using configurations placement
/// index.
static const uint8_t VIB_BLE_CFG_AUTO_ID_FLAG = (1 << 0);

/// If set on device config, will use provided 8-bit UUID to generate 128-bit
/// UUID.
static const uint8_t VIB_BLE_CFG_8BIT_ID_FLAG = (1 << 1);

// -------------------------------------------------------------
// Client-Oriented Transparent Structures
// -------------------------------------------------------------

/**
 * @brief       Characteristic configuration structure with control-callbacks.
 */
typedef struct {
    void (*on_read)(void *);  ///< Called on characteristic read request.
    void (*on_write)(void *); ///< Called on characteristic write request.

    uint8_t uuid8;                     ///< 8-bit Characteristic ID.
    uint8_t uuid128[ESP_UUID_LEN_128]; ///< 128-bit Characteristic ID.
                                       ///< Generated from 8-bit ID if provided.

} vib_ble_cfg_chr_t;

/**
 * @brief       Service configuration structure.
 */
typedef struct {
    uint8_t chr_tab_len; ///< Number of characteristics
    vib_ble_cfg_chr_t
        *chr_tab; ///< Array of characteristic control-callback containers.

    uint8_t uuid8;                     ///< 8-bit Service ID.
    uint8_t uuid128[ESP_UUID_LEN_128]; ///< 128-bit Service ID. Generated from
                                       ///<     8-bit ID if provided.

} vib_ble_cfg_svc_t;

/**
 * @brief       Device configuration structure.
 *
 * Note:
 *     This and other structures are intended to be defined manually and passed
 *     to top-level domain procedures.
 */
typedef struct {
    char *name;       ///< Device name, this is what is advertised to central
    uint8_t name_len; ///< Length of the device name.

    char *mfr;       ///< Manufacturer name.
    uint8_t mfr_len; ///< Length of the manufacturer name.

    vib_ble_cfg_svc_t
        *svc_tab; ///< Service table, array of service configuration containers.
    uint8_t svc_tab_len;

    uint8_t flags; ///< Configuration Flags.

} vib_ble_cfg_dev_t;

// -------------------------------------------------------------
// Auxiliary Procedures
// -------------------------------------------------------------

/**
 * @brief       Determine length of required handle buffer.
 */
uint8_t vib_ble_cfg_handle_range(const vib_ble_cfg_dev_t *const dev_cfg);

// -------------------------------------------------------------
// Characteristic Handle-To-Configuration Map
// -------------------------------------------------------------

/**
 * @brief       Primitive handle-to-char_config mapping structure.
 *
 *              Used to relate handles on read/write requests to their
 *              appropriate configuration structure.
 */
typedef struct vib_ble_cfg_chr_map_s vib_ble_cfg_chr_map_t;

vib_ble_cfg_chr_map_t *
vib_ble_cfg_chr_map_init(const vib_ble_cfg_dev_t *dev_cfg,
                         uint16_t *handle_buffer, uint16_t handle_buffer_len);
void vib_ble_cfg_chr_map_deinit(vib_ble_cfg_chr_map_t *map);
void vib_ble_cfg_chr_map_set(vib_ble_cfg_chr_map_t *map, uint16_t handle,
                             vib_ble_cfg_chr_t *value);
vib_ble_cfg_chr_t *vib_ble_cfg_chr_map_get(vib_ble_cfg_chr_map_t *map,
                                           uint16_t key);

#endif // VIB_BLE_CFG_H_
