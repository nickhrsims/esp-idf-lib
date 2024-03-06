// SPDX-FileCopyrightText: 2023 Nicholas H.R. Sims <nickhrsims@gmail.com>
//
// SPDX-License-Identifier: Apache-2.0

/// neil_ble_gatts_cfg.h
///
/// @author     Nicholas H.R. Sims
///
/// @brief      Bluetooth Low-Energy Domain API.

#ifndef neil_ble_gatts_CFG_H_
#define neil_ble_gatts_CFG_H_

#include "esp_bt_defs.h"

// -------------------------------------------------------------
// Generic UUID System
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
//     neil_ble_gatts_UUID_128(service_index, characteristic_index)
//
// @see https://www.uuidgenerator.net/
#define neil_ble_gatts_UUID_128(XX, YY)                                               \
    {                                                                          \
        0xD7, 0x36, 0x7A, 0x53, 0x0C, 0x0A, 0xD1, 0x84, 0x2E, 0x45, YY, XX,    \
            0xD6, 0xB9, 0xD5, 0xC2                                             \
    }

/// Get service index (by-convention) from UUID.
#define neil_ble_gatts_UUID_128_GET_SVC_INDEX(uuid128) uuid128[11]

/// Get characteristic index (by-convention) from UUID.
#define neil_ble_gatts_UUID_128_GET_CHR_INDEX(uuid128) uuid128[10]

// -------------------------------------------------------------
// Device Configuration Structures
// -------------------------------------------------------------

/**
 * @brief       Characteristic configuration structure with control-callbacks.
 */
typedef struct {
    void (*on_read)(uint8_t *data);               ///< Read callback
    void (*on_write)(uint8_t *val, uint16_t len); ///< Write callback

    uint16_t size; ///< Data size for read/write operations.

    uint8_t uuid[ESP_UUID_LEN_128]; ///< 128-bit Characteristic ID.

} neil_ble_gatts_cfg_chr_t;

/**
 * @brief       Service configuration structure.
 */
typedef struct {
    uint8_t chr_tab_len; ///< Number of characteristics
    neil_ble_gatts_cfg_chr_t
        *chr_tab; ///< Array of characteristic control-callback containers.

    uint8_t uuid[ESP_UUID_LEN_128]; ///< 128-bit Service ID.

} neil_ble_gatts_cfg_svc_t;

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

    neil_ble_gatts_cfg_svc_t
        *svc_tab; ///< Service table, array of service configuration containers.
    uint8_t svc_tab_len;

} neil_ble_gatts_cfg_dev_t;

#endif // neil_ble_gatts_CFG_H_
