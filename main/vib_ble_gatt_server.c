/// vib_ble_gatt_server.c

#include <stdint.h>
#include <string.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gatt_defs.h"
#include "esp_gatts_api.h"
#include "esp_log.h"

#include "vib_ble_cfg.h"
#include "vib_ble_gap.h"
#include "vib_ble_gatt_server.h"
#include "vib_ble_gatt_table.h"

// -------------------------------------------------------------
// Settings
// -------------------------------------------------------------

static const char *const TAG = "VIB_BLE_GATT_SERVER";

// Specific instance of the primary service.
//
// NOTE: Only one instance will exist at a time, thus is constant.
// static const uint8_t INSTANCE_ID = 0;

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
static const vib_ble_cfg_dev_t *device_config = NULL;

// -------------------------------------------------------------
// Prototypes
// -------------------------------------------------------------

// --- Events
static void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param);

// -------------------------------------------------------------
// Dependency Management
// -------------------------------------------------------------

/**
 * @brief       Set device configuration structure used by the GATT Server.
 */
static void device_config_set(const vib_ble_cfg_dev_t *const dev_cfg) {
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
void vib_ble_gatt_server_start(const vib_ble_cfg_dev_t *dev_cfg) {

    // --- Prepare Device Configuration
    device_config_set(dev_cfg);

    // --- Return Code for repeat checks
    esp_err_t ret;

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
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s init controller failed: %s", __func__,
                 esp_err_to_name(ret));
        return;
    }

    // --- Enable BT Controller
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed: %s", __func__,
                 esp_err_to_name(ret));
        return;
    }

    // ---------------------------------
    // Bluedroid Stack
    // ---------------------------------

    // --- Initialize Bluedroid Stack
    ESP_LOGI(TAG, "%s init bluetooth", __func__);
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluetooth failed: %s", __func__,
                 esp_err_to_name(ret));
        return;
    }

    // --- Enable Bluedroid Stack
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "%s enable bluetooth failed: %s", __func__,
                 esp_err_to_name(ret));
        return;
    }

    // ---------------------------------
    // Callback Registration
    // ---------------------------------

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret) {
        ESP_LOGE(TAG, "gatts register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gap_register_callback(vib_ble_gap_event_handler);
    if (ret) {
        ESP_LOGE(TAG, "gap register error, error code = %x", ret);
        return;
    }

    // ---------------------------------
    // Application Profile Registration
    // ---------------------------------

    ret = esp_ble_gatts_app_register(PROFILE_ID);
    if (ret) {
        ESP_LOGE(TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    // ---------------------------------
    // Configure GAP Security Parameters
    // ---------------------------------
    vib_ble_gap_configure_security();

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
    vib_ble_cfg_chr_t **data;
} handle_map_t;

/**
 * @brief       Allocate and configure a new handle-to-config map.
 */
static handle_map_t *handle_map_init(const vib_ble_cfg_dev_t *dev_cfg,
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

    handle_map_t *map = malloc(sizeof(handle_map_t));

    map->len    = handle_buffer_len;
    map->offset = handle_space_offset;

    // Initialize the internal mapping table with
    // length * the size of a characteristic config pointer.
    map->data = malloc(map->len * sizeof(vib_ble_cfg_chr_t *));

    for (uint8_t svc_idx = 0; svc_idx < dev_cfg->svc_tab_len; svc_idx++) {
        attr_idx += svc_offset;

        // --- Acquire Service Config
        vib_ble_cfg_svc_t *svc_cfg = (dev_cfg->svc_tab + svc_idx);

        for (uint8_t chr_idx = 0;
             chr_idx < (dev_cfg->svc_tab + svc_idx)->chr_tab_len; chr_idx++) {

            attr_idx += attr_val_offset;

            vib_ble_cfg_chr_t *chr_cfg = (svc_cfg->chr_tab + chr_idx);

            *(map->data + attr_idx) = chr_cfg;
        }
    }

    return map;
}

/**
 * @brief       Get configuration by handle from a map.
 */
static vib_ble_cfg_chr_t *handle_map_get(handle_map_t *map, uint16_t handle) {
    return *(map->data + (handle - map->offset));
}

// HACK: Error Checking
static void handle_map_deinit(handle_map_t *map) {

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
//    vib_ble_gap_init();

// FIXME: Documentation
static void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param) {

    static const uint8_t INSTANCE_ID = 0;
    static vib_ble_gatt_table_t *attr_tab;
    static handle_map_t *handle_map;

    switch (event) {

    // ---------------------------------
    // Configuration Events
    // ---------------------------------

    //
    // --- On Application (Profile) ID Registration
    //
    case ESP_GATTS_REG_EVT:

        // --- Prepare GAP
        // FIXME: Violates abstraction level, move name into gap init!
        esp_ble_gap_set_device_name(device_config->name);
        vib_ble_gap_init(device_config);

        // --- Configure Privacy Settings
        //
        // NOTE: This will trigger the remaining GAP setup chain
        esp_ble_gap_config_local_privacy(true);

        ESP_LOGI(TAG, "Initializing GATT Table");

        // --- Prepate Attribute Table
        attr_tab = vib_ble_gatt_table_init(device_config);

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

        handle_map = handle_map_init(device_config, param->add_attr_tab.handles,
                                     param->add_attr_tab.num_handle);

        ESP_LOGI(TAG, "Handle Mapping Created");

        uint8_t attr_idx = -1;

        // --- Start Services
        //     FIXME: Factor out into abstraction-level appropriate call
        for (uint8_t svc_idx = 0; svc_idx < device_config->svc_tab_len;
             svc_idx++) {
            attr_idx += attr_svc_offset;

            vib_ble_cfg_svc_t *svc_cfg = device_config->svc_tab + svc_idx;

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
        ESP_LOGI(TAG, "Read: Handle(%x)", param->read.handle);
        // Acquire characteristic config object
        vib_ble_cfg_chr_t *chr_cfg =
            handle_map_get(handle_map, param->read.handle);

        // Prepare response object
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len    = chr_cfg->size;

        // Read data into response object
        chr_cfg->on_read(rsp.attr_value.value);

        // Send response
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id,
                                    param->read.trans_id, ESP_GATT_OK, &rsp);
        break;
    }

        // ---------------------------------
        // Data Write Events
        // ---------------------------------

        //
        // --- On Write Operation Request
        //
    case ESP_GATTS_WRITE_EVT: {
        ESP_LOGI(TAG, "ESP_GATTS_WRITE_EVT, write value:");
        esp_log_buffer_hex(TAG, param->write.value, param->write.len);

        handle_map_get(handle_map, param->write.handle)
            ->on_write(param->write.value, param->write.len);

        break;
    }

    //
    // --- On Application (Profile) ID Un-registration
    //
    case ESP_GATTS_UNREG_EVT:
        handle_map_deinit(handle_map);
        vib_ble_gatt_table_deinit(attr_tab);
        break;

    // ---------------------------------
    // Connection Events
    // ---------------------------------

    // --- On Client Connection
    case ESP_GATTS_CONNECT_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_CONNECT_EVT");
        /* start security connect with peer device when receive the connect
         * event sent by the master */
        esp_ble_set_encryption(param->connect.remote_bda,
                               ESP_BLE_SEC_ENCRYPT_MITM);
        break;

    // --- On Client Disconnection
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x",
                 param->disconnect.reason);
        /* start advertising again when missing the connect */
        vib_ble_gap_advertise();
        break;

    // --- On Connection Listener Started
    case ESP_GATTS_LISTEN_EVT:
        break;

    //
    // --- On Write Operation Confirmation Request
    //
    // NOTE: Assumed write operation relationships
    //
    //       This does not appear to be necessary in the current model,
    //       as the pdu size is small enough for each characteristic.
    //
    // [client]         [server]
    //    |    ------->    |
    //    |  request write |
    //    |                |
    //    |   <-------     |
    //    | respond ok/err |
    //    |                |
    //    |   if got ok    |
    //    |    ------->    |
    //    |  request exec  |
    //    |                |
    //    |            --- |
    //    | do write  |    |
    //    |            --> |
    case ESP_GATTS_EXEC_WRITE_EVT:
        break;

    // ---------------------------------
    // Service Events
    // ---------------------------------

    // --- On Service Deleted
    case ESP_GATTS_DELETE_EVT:
        break;

    // --- On Service Started
    case ESP_GATTS_START_EVT:
        break;

    // --- On Service Stopped
    case ESP_GATTS_STOP_EVT:
        break;

    // ---------------------------------
    // Server-Specific Events
    // ---------------------------------

    // --- On Peer Connected
    case ESP_GATTS_OPEN_EVT:
        break;

    // --- On Peer Disconnected
    case ESP_GATTS_CANCEL_OPEN_EVT:
        break;

    // --- On Server Closed
    case ESP_GATTS_CLOSE_EVT:
        break;

    // --- On Server Congestion
    case ESP_GATTS_CONGEST_EVT:
        break;

    // ---------------------------------
    // Other Events
    // ---------------------------------

    // --- On Set MTU
    case ESP_GATTS_MTU_EVT:
        break;

    // --- On General Confirmation Event
    case ESP_GATTS_CONF_EVT:
        break;

    default:
        break;
    }
}