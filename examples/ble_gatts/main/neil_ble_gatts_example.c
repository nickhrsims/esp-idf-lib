// SPDX-FileCopyrightText: 2023 Nicholas H.R. Sims <nickhrsims@gmail.com>
//
// SPDX-License-Identifier: Apache-2.0

/// @brief       Application entry-point.
/// @author      Nicholas H.R. Sims

#include <stdint.h>
#include <string.h>

#include "esp_log.h"

#include "neil_ble_gatts.h"
#include "neil_ble_gatts_cfg.h"

#include "neil_ble_gatts_example.h"

/// Advertised device name.
#define BLE_DEVICE_NAME "EXAMPLE"

/// Advertised device manufacturer.
#define BLE_MFR_NAME "EXAMPLE"

/// Default value for Attribute 0.
#define ATTR_0_DEFAULT_VALUE 0

/// Logging tag.
static const char *TAG = "NEIL BLE GATTS Example App";

/**
 * @brief       Attribute 0 Data-Transfer Object.
 *
 *              Provides zero-cast translation between raw bytes and value data-type.
 */
typedef union {
    uint8_t raw[sizeof(float)];
    float value;
} attr_0_dto_t;

/**
 * @brief       Callback to-be-registered for reading an attribute.
 *
 *              Use one per-attribute.
 */
void read_attr_0(uint8_t *buffer) {
    attr_0_dto_t attr_0_dto = {.value = ATTR_0_DEFAULT_VALUE};
    memcpy(buffer, attr_0_dto.raw, sizeof(float));
    ESP_LOGI(TAG, "Attribute 0 Read: Type(float) Value(%f)", attr_0_dto.value);
}

/**
 * @brief       Callback to-be-registered for writing an attribute.
 *
 *              Use one per-attribute.
 */
void write_attr_0(uint8_t *data, uint16_t len) {
    attr_0_dto_t attr_0_dto;
    memcpy(attr_0_dto.raw, data, len);
    ESP_LOGI(TAG, "Attribute 0 Write: Type(float) Value(%f)", attr_0_dto.value);
}

// --- Top-level device configuration.
//
// @see: neil_ble_gatts_cfg.h
static neil_ble_gatts_cfg_dev_t bluetooth_device_config = {

    .name_len = sizeof(BLE_DEVICE_NAME),
    .name     = BLE_DEVICE_NAME,

    .mfr_len = sizeof(BLE_MFR_NAME),
    .mfr     = BLE_MFR_NAME,

    .svc_tab_len = 1,
    .svc_tab =

        (neil_ble_gatts_cfg_svc_t[]){

            // --- Control Service ---
            {
                .uuid = APP_BLE_UUID_128(0, 0),

                .chr_tab_len = 1,
                .chr_tab =
                    (neil_ble_gatts_cfg_chr_t[]){

                        // --- Gain ---
                        {
                            .uuid = APP_BLE_UUID_128(0, 1),

                            .size = sizeof(float),

                            .on_read  = read_attr_0,
                            .on_write = write_attr_0,
                        },
                    },
            },
        },

};

/**
 * @brief       Application entry-point.
 */
void app_main(void) {
    // Start a new Bluetooth Low-Energy GATT Server using the above specified
    // config.
    neil_ble_gatts_start(&bluetooth_device_config);
}
