// SPDX-FileCopyrightText: 2023 Nicholas H.R. Sims <nickhrsims@gmail.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <string.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gatt_defs.h"
#include "esp_gatts_api.h"
#include "esp_log.h"

#include "neil_ble_gatts.h"
#include "neil_ble_gatts_attr_db.h"
#include "neil_ble_gatts_cfg.h"
#include "neil_ble_gatts_gap.h"

// -------------------------------------------------------------
// Settings
// -------------------------------------------------------------

static const char *const TAG = "NEIL BLE GATTS";

// Application Profile ID
//
// NOTE: This implementation supports only one application profile.
static const uint8_t PROFILE_ID = 0;

// -------------------------------------------------------------
// Dependencies
// -------------------------------------------------------------

// Device Configuration
//
// NOTE: Must be set by dependency management procedures
static const neil_ble_gatts_cfg_dev_t *device_config = NULL;

// -------------------------------------------------------------
// Prototypes
// -------------------------------------------------------------

// --- Events
static void gatts_event_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                 esp_ble_gatts_cb_param_t *param);

// -------------------------------------------------------------
// Dependency Management
// -------------------------------------------------------------

/**
 * @brief       Set device configuration structure used by the GATT Server.
 */
static void device_config_set(const neil_ble_gatts_cfg_dev_t *const dev_cfg) {
    device_config = dev_cfg;
}

/**
 * @brief       Clear device configuration structure used by the GATT Server.
 *
 *              Warning: If the GATT Server is still running when this is
 *                       called, it is considered undefined behavior.
 */
static void device_config_clear() { device_config = NULL; }

// -------------------------------------------------------------
// Initialization / Deinitialization
// -------------------------------------------------------------

// FIXME: Documentation
void neil_ble_gatts_start(const neil_ble_gatts_cfg_dev_t *dev_cfg) {

    // --- Prepare Device Configuration
    device_config_set(dev_cfg);

    // ---------------------------------
    // Memory Release
    // ---------------------------------

    // --- Release Heap Memory from unused bluetooth mode
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // ---------------------------------
    // Bluetooth Controller
    // ---------------------------------

    // --- Prepare Default BTC Config
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    // --- Initialize BT Controller
    esp_bt_controller_init(&bt_cfg);

    // --- Enable BT Controller
    esp_bt_controller_enable(ESP_BT_MODE_BLE);

    // ---------------------------------
    // Bluedroid Stack
    // ---------------------------------

    // --- Initialize Bluedroid Stack
    esp_bluedroid_init();

    // --- Enable Bluedroid Stack
    esp_bluedroid_enable();

    // ---------------------------------
    // Callback Registration
    // ---------------------------------

    esp_ble_gatts_register_callback(gatts_event_callback);

    esp_ble_gap_register_callback(neil_ble_gatts_gap_event_handler);

    // ---------------------------------
    // Application Profile Registration
    // ---------------------------------

    esp_ble_gatts_app_register(PROFILE_ID);

    // ---------------------------------
    // Configure GAP Security Parameters
    // ---------------------------------
    neil_ble_gatts_gap_configure_security();

    return;
}

// -------------------------------------------------------------
// Characteristic-Handle-to-Configuration Map
// -------------------------------------------------------------

/**
 * @brief       Primitive handle-to-char_config mapping structure.
 *
 *              Used to relate handles on read/write requests to their
 *              appropriate configuration structure.
 */
typedef struct {
    size_t len;
    uint16_t offset;
    neil_ble_gatts_cfg_chr_t **data;
} chr_handle_map_t;

/**
 * @brief       Allocate and configure a new handle-to-config map.
 */
static chr_handle_map_t *chr_handle_map_init(const neil_ble_gatts_cfg_dev_t *dev_cfg,
                                             uint16_t *handle_buffer,
                                             uint16_t handle_buffer_len) {

    // --- Attributes per characteristic.
    //
    // NOTE: This information is implicit in the design structure of the GATT
    //       table.
    static const uint8_t attr_val_offset = 2;

    // --- Services only require one attribute.
    static const uint8_t svc_offset = 1;

    const uint16_t handle_space_offset = handle_buffer[0];

    uint8_t attr_idx = -1;

    chr_handle_map_t *map = malloc(sizeof(chr_handle_map_t));

    map->len    = handle_buffer_len;
    map->offset = handle_space_offset;

    // Initialize the internal mapping table with
    // length * the size of a characteristic config pointer.
    map->data = malloc(map->len * sizeof(neil_ble_gatts_cfg_chr_t *));

    for (uint8_t svc_idx = 0; svc_idx < dev_cfg->svc_tab_len; svc_idx++) {
        attr_idx += svc_offset;

        // --- Acquire Service Config
        neil_ble_gatts_cfg_svc_t *svc_cfg = (dev_cfg->svc_tab + svc_idx);

        for (uint8_t chr_idx = 0; chr_idx < (dev_cfg->svc_tab + svc_idx)->chr_tab_len;
             chr_idx++) {

            attr_idx += attr_val_offset;

            neil_ble_gatts_cfg_chr_t *chr_cfg = (svc_cfg->chr_tab + chr_idx);

            *(map->data + attr_idx) = chr_cfg;
        }
    }

    return map;
}

/**
 * @brief       Get configuration by handle from a map.
 */
static neil_ble_gatts_cfg_chr_t *chr_handle_map_get(chr_handle_map_t *map,
                                                    uint16_t handle) {
    return *(map->data + (handle - map->offset));
}

// HACK: Error Checking
static void chr_handle_map_deinit(chr_handle_map_t *map) {

    // Set all values in data array to null.
    size_t len = map->len;
    for (uint8_t idx = 0; idx < len; ++idx) {
        *(map->data + idx) = NULL;
    }

    free(map->data);
    free(map);
}

// -------------------------------------------------------------
// GATT Server Event Management
// -------------------------------------------------------------

//    attribute_table_init();
//    neil_ble_gatts_gap_init();

// FIXME: Documentation
static void gatts_event_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                 esp_ble_gatts_cb_param_t *param) {

    static const uint8_t INSTANCE_ID = 0;
    static neil_ble_gatts_attr_db_t *attr_tab;
    static chr_handle_map_t *handle_map;

    switch (event) {

    // ---------------------------------
    // Configuration Events
    // ---------------------------------

    //
    // --- On Application (Profile) ID Registration
    //
    case ESP_GATTS_REG_EVT:

        // --- Prepare GAP
        neil_ble_gatts_gap_init(device_config);

        // --- Configure Privacy Settings
        //
        // NOTE: This will trigger the remaining GAP setup chain
        esp_ble_gap_config_local_privacy(true);

        ESP_LOGI(TAG, "Initializing GATT Table");

        // --- Prepate Attribute Table
        attr_tab = neil_ble_gatts_attr_db_init(device_config);

        // FIXME: Wrap table creation;
        esp_ble_gatts_create_attr_tab(attr_tab->data, gatts_if, attr_tab->len,
                                      INSTANCE_ID);
        break;

    //
    // --- On GATTS Attribute Table "Creation"
    //
    case ESP_GATTS_CREAT_ATTR_TAB_EVT: {

        // FIXME: Move out into global knowledge header,
        //        maybe from the gatt table authority
        static uint8_t attr_svc_offset = 1;
        static uint8_t attr_chr_offset = 2;

        ESP_LOGI(TAG, "Attribute Table Created");

        handle_map = chr_handle_map_init(device_config, param->add_attr_tab.handles,
                                         param->add_attr_tab.num_handle);

        ESP_LOGI(TAG, "Handle Mapping Created");

        uint8_t attr_idx = -1;

        // --- Start Services
        //     FIXME: Factor out into abstraction-level appropriate call
        for (uint8_t svc_idx = 0; svc_idx < device_config->svc_tab_len; svc_idx++) {
            attr_idx += attr_svc_offset;

            neil_ble_gatts_cfg_svc_t *svc_cfg = device_config->svc_tab + svc_idx;

            uint16_t svc_handle = attr_idx + handle_map->offset;

            ESP_LOGI(TAG, "Starting Service Handle: %x", svc_handle);
            esp_ble_gatts_start_service(svc_handle);

            attr_idx += svc_cfg->chr_tab_len * attr_chr_offset;
        }

        ESP_LOGI(TAG, "FINISHED STARTING SERVICSE");

        break;
    }

        // ---------------------------------
        // Data Read Events
        // ---------------------------------

        //
        // --- On Read Operation Request
        //
    case ESP_GATTS_READ_EVT: {

        // Acquire characteristic config object
        neil_ble_gatts_cfg_chr_t *chr_cfg =
            chr_handle_map_get(handle_map, param->read.handle);

        // Prepare response object
        esp_gatt_rsp_t rsp;

        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));

        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len    = chr_cfg->size;

        // Read data into response object
        chr_cfg->on_read(rsp.attr_value.value);

        // Send response
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);
        break;
    }

    // ---------------------------------
    // Data Write Events
    // ---------------------------------

    //
    // --- On Write Operation Request
    //
    case ESP_GATTS_WRITE_EVT: {
        esp_log_buffer_hex(TAG, param->write.value, param->write.len);
        chr_handle_map_get(handle_map, param->write.handle)
            ->on_write(param->write.value, param->write.len);
        break;
    }

    //
    // --- On Application (Profile) ID Un-registration
    //
    case ESP_GATTS_UNREG_EVT:
        chr_handle_map_deinit(handle_map);
        neil_ble_gatts_attr_db_deinit(attr_tab);
        break;

    // ---------------------------------
    // Connection Events
    // ---------------------------------

    // --- On Client Connection
    case ESP_GATTS_CONNECT_EVT:
        esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
        break;

    // --- On Client Disconnection
    case ESP_GATTS_DISCONNECT_EVT:
        neil_ble_gatts_gap_advertise();
        break;

    default:
        break;
    }
}
