// SPDX-FileCopyrightText: 2023 Nicholas H.R. Sims <nickhrsims@gmail.com>
//
// SPDX-License-Identifier: Apache-2.0

/// neil_ble_gatts_attr_db.c
///
/// @author     Nicholas H.R. Sims
///
/// @brief      GATT Attribute Table implementation wrapper.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "esp_gatt_defs.h"

#include "esp_log.h"
#include "neil_ble_gatts_cfg.h"
#include "neil_ble_gatts_attr_db.h"

static const char *TAG = "neil_ble_gatts_attr_db";

// -------------------------------------------------------------
// Attribute Handle Utilities
// -------------------------------------------------------------

/**
 * @brief       Get the length of memory required for a GATT table
 *              based on a device configuration structure.
 *
 *              This length is also used to hold attribute handles,
 *              and can be used to determine how many 2-byte memory cells are
 *              needed to create a handle-to-configuration-entry map.
 */
static uint8_t handle_buffer_range(const neil_ble_gatts_cfg_dev_t *const dev_cfg) {

    // --- 2-byte wide handles (uint16_t)
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

/**
 * @brief       Determine offset of handle-space that includes attributes.
 */
static uint16_t handle_buffer_offset(uint16_t *handle_buffer) {
    return handle_buffer[0];
}

// -------------------------------------------------------------
// Attribute Table Management
// -------------------------------------------------------------

// ---------------------------------
// SIG Adopted Attribute Type UUIDs
// ---------------------------------
//
// These 16-bit UUID fields mark the type of attribute being registered.
//
// All are documented, standardized values that can be found in the accompanying
// citation.
//
// @see
// https://bitbucket.org/bluetooth-SIG/public/src/main/assigned_numbers/uuids

// --- Generic Type Attribute UUIDs (Little-Endian)

/// Primary Service Declaration Type UUID.
/// Used to specify the attribute is of type
/// "Primary Service Declaration" in a
/// GATT DB Table.
static uint8_t SVC_TYPE_UUID[2] = {0x00, 0x28};
/// Characteristic Declaration Type UUID.
/// Used to specify the attribute is of type
/// "Characteristic Declaration" in a
/// GATT DB Table.
static uint8_t CHR_TYPE_UUID[2] = {0x03, 0x28};

/* --- Unused Type UUID Constants (left here for documentation)
static const uint16_t __attribute__((unused))
CHARACTERISTIC_CLIENT_CONFIGURATION_TYPE_UUID = 0x2902;

static const uint16_t __attribute__((unused))
SECONDARY_SERVICE_TYPE_UUID = 0x2801;

static const uint16_t __attribute__((unused))
INCLUDE_SERVICE_TYPE_UUID = 0x2802;
*/

// ---------------------------------
// Auxiliary
// ---------------------------------

// The size of a characteristics declaration data is one byte
static uint8_t CHR_DECL_SIZE = sizeof(uint8_t);

// Read/Write Property Flag
static uint8_t CHR_PROP_FLAGS =
    ESP_GATT_CHAR_PROP_BIT_WRITE_NR | ESP_GATT_CHAR_PROP_BIT_READ;

// ---------------------------------
// Initialization
// ---------------------------------

neil_ble_gatts_attr_db_t *
neil_ble_gatts_attr_db_init(const neil_ble_gatts_cfg_dev_t *dev_cfg) {

    // ---------------------------------
    // Initialization
    // ---------------------------------

    ESP_LOGI(TAG, "Initializing Table");

    neil_ble_gatts_attr_db_t *attr_tab = malloc(sizeof(neil_ble_gatts_attr_db_t));

    // Attribute Table Index (moved by the loop control)
    uint8_t attr_idx = 0;

    // Capture the length of the table
    attr_tab->len = handle_buffer_range(dev_cfg);

    // Generic Attributes Table
    // TODO: Error Check
    attr_tab->data = malloc(attr_tab->len * sizeof(esp_gatts_attr_db_t));

    // Service Table
    const neil_ble_gatts_cfg_svc_t *svc_tab = dev_cfg->svc_tab;
    // Number of Services in the Service Table
    const uint8_t svc_len = dev_cfg->svc_tab_len;

    // ---------------------------------
    // For Each Service
    // ---------------------------------
    // TODO Error Checks
    for (uint8_t svc_idx = 0; svc_idx < svc_len; svc_idx++) {

        // ---------------------------------
        // Prepare the Values
        // ---------------------------------

        // Current Service Config
        const neil_ble_gatts_cfg_svc_t *svc_cfg = svc_tab + svc_idx;

        // Characteristics Table
        const neil_ble_gatts_cfg_chr_t *chr_tab = svc_cfg->chr_tab;

        // Number of Characteristic Table Elements
        const uint8_t chr_len = svc_cfg->chr_tab_len;

        // Service ID
        const uint8_t *svc_id = svc_cfg->uuid;

        // ---------------------------------
        // Construct the Service Attribute
        // ---------------------------------

        ESP_LOGI(TAG, "Preparing Service Attribute (%d)", svc_idx);

        *(attr_tab->data + attr_idx++) = (esp_gatts_attr_db_t){
            .attr_control = {.auto_rsp = ESP_GATT_AUTO_RSP},
            .att_desc =
                // For Service Declarations:
            {
                // The UUID fields are used to specify the GATT type
                // in this case, service.
                .uuid_length = ESP_UUID_LEN_16,
                .uuid_p      = SVC_TYPE_UUID,

                // Service declarations at minimum must be read.
                .perm = ESP_GATT_PERM_READ,

                // The value field contains the actual service UUID.
                .max_length = ESP_UUID_LEN_128,
                .length     = ESP_UUID_LEN_128,
                .value      = (uint8_t *)svc_id,
            },
        };

        // ----------------------------------------------
        // For Each Characteristic in the current Service
        // ----------------------------------------------
        for (uint8_t chr_idx = 0; chr_idx < chr_len; chr_idx++) {

            const neil_ble_gatts_cfg_chr_t *chr_cfg = (chr_tab + chr_idx);

            const uint8_t *chr_id = chr_cfg->uuid;

            ESP_LOGI(TAG, "Preparing Characteristic Attribute (%d/%d)", svc_idx,
                     chr_idx);

            // ---------------------------------
            // Construct Declaration Attribute
            // ---------------------------------

            *(attr_tab->data + attr_idx++) = (esp_gatts_attr_db_t){
                .attr_control = {.auto_rsp = 0},
                .att_desc =
                    // For Characteristic Declarations:
                {
                    // The UUID fields specifiy the GATT type,
                    // in this case, characteristic.
                    .uuid_length = ESP_UUID_LEN_16,
                    .uuid_p      = CHR_TYPE_UUID,
                    // Characteristic Declarations must be readable.
                    .perm = ESP_GATT_PERM_READ,
                    // The values are the properties of the characteristic.
                    .max_length = CHR_DECL_SIZE,
                    .length     = CHR_DECL_SIZE,
                    // FIXME: Support parameterized config
                    .value = &CHR_PROP_FLAGS,
                },
            };

            // ---------------------------------
            // Construct Value Attribute
            // ---------------------------------

            *(attr_tab->data + attr_idx++) = (esp_gatts_attr_db_t){
                .attr_control = {.auto_rsp = 0},
                .att_desc =
                    // For Characteristic Values:
                {
                    // The UUID fields directly relate to the real UUID of the
                    // attribute.
                    .uuid_length = ESP_UUID_LEN_128,
                    .uuid_p      = (uint8_t *)chr_id,
                    // Permissions should match declaration properties.
                    // FIXME: Support parameterized config
                    .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                    // Internal value buffers are unused in this implementation
                    .max_length = 0,
                    .length     = 0,
                    .value      = NULL,
                },
            };
        }
    }

    return attr_tab;
}

// ---------------------------------
// Termination
// ---------------------------------

void neil_ble_gatts_attr_db_deinit(neil_ble_gatts_attr_db_t *attr_tab) {
    free(attr_tab->data);
    attr_tab->len = 0;
    free(attr_tab);
}
