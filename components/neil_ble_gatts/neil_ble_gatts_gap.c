#include <string.h>

#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_log.h"

#include "neil_ble_gatts_cfg.h"
#include "neil_ble_gatts_gap.h"
#include "neil_ble_gatts_util.h"

static const char *const TAG = "neil_ble_gatts_GAP";

static struct gap_config_s {
    esp_ble_adv_data_t adv_data;
    esp_ble_adv_data_t adv_ext_data;
    esp_ble_adv_params_t adv_params;
} gap_config;

// -------------------------------------------------------------
// Prototypes
// -------------------------------------------------------------

// --- Advertising
static void adv_svc_uuid_merge(const neil_ble_gatts_cfg_dev_t *dev_cfg, uint8_t *uuid);

// -------------------------------------------------------------
// Advertising State Control Flags
// -------------------------------------------------------------
// --- Informs advertising system is configured
static const uint8_t ADV_CONFIG_COMPLETED_FLAG = 0b01;
// --- Informs Scan Response system is configured
static const uint8_t SCAN_RSP_CONFIG_COMPLETED_FLAG = 0b10;
// --- Condition byte used to track status flags
static uint8_t is_adv_config_done = 0;

void neil_ble_gatts_gap_init(const neil_ble_gatts_cfg_dev_t *dev_cfg) {

    // FIXME: Increase size to maximum length
    static uint8_t adv_svc_uuid[16];

    // Initialize the advertising ID with appropriate size
    adv_svc_uuid_merge(dev_cfg, adv_svc_uuid);

    // For each service in the device config
    for (uint8_t svc_idx = 0; svc_idx < dev_cfg->svc_tab_len; svc_idx++) {
        uint16_t offset = ESP_UUID_LEN_128 * svc_idx;
        // Create a UUID to relate to the service
        memcpy(&adv_svc_uuid[offset], (uint8_t[])neil_ble_gatts_UUID_128(svc_idx, 0),
               ESP_UUID_LEN_128);
    }

    gap_config = (struct gap_config_s){
        .adv_data =
            {
                .set_scan_rsp    = false,
                .include_txpower = true,
                .min_interval = 0x0006, // slave connection min interval, Time =
                                        // min_interval * 1.25 msec
                .max_interval = 0x0010, // slave connection max interval, Time =
                                        // max_interval * 1.25 msec
                .appearance          = 0x00,
                .manufacturer_len    = 0,
                .p_manufacturer_data = NULL,
                .service_data_len    = 0,
                .p_service_data      = NULL,
                .service_uuid_len    = sizeof(adv_svc_uuid),
                .p_service_uuid      = adv_svc_uuid,
                .flag                = (ESP_BLE_ADV_FLAG_GEN_DISC |
                         ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
            },

        .adv_ext_data =
            {
                .set_scan_rsp        = true,
                .include_name        = true,
                .manufacturer_len    = dev_cfg->mfr_len,
                .p_manufacturer_data = (uint8_t *)dev_cfg->mfr,
            },

        .adv_params =
            {
                .adv_int_min       = 0x100,
                .adv_int_max       = 0x100,
                .adv_type          = ADV_TYPE_IND,
                .own_addr_type     = BLE_ADDR_TYPE_RPA_PUBLIC,
                .channel_map       = ADV_CHNL_ALL,
                .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
            },
    };
}

void neil_ble_gatts_gap_advertise() {
    esp_ble_gap_start_advertising(&gap_config.adv_params);
}

/**
 * @brief       Handle incoming GAP Events.
 *
 *              Responds to BLE GAP Events that occur during advertising and
 *              response procedures.
 */
void neil_ble_gatts_gap_event_handler(esp_gap_ble_cb_event_t event,
                               esp_ble_gap_cb_param_t *param) {

    ESP_LOGV(TAG, "GAP_EVT, event %d\n", event);

    switch (event) {

    // --- On Advertisement Config Done
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        is_adv_config_done &= (~ADV_CONFIG_COMPLETED_FLAG);
        if (is_adv_config_done == 0) {
            neil_ble_gatts_gap_advertise();
        }
        break;

    // --- On Response Config Done
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        is_adv_config_done &= (~SCAN_RSP_CONFIG_COMPLETED_FLAG);
        if (is_adv_config_done == 0) {
            neil_ble_gatts_gap_advertise();
        }
        break;

    // --- On Advertisement Start
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        // advertising start complete event to indicate advertising start
        // successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(TAG, "advertising start failed, error status = %x",
                     param->adv_start_cmpl.status);
            break;
        }
        ESP_LOGI(TAG, "advertising start success");
        break;

    // --- On Passkey Request (ingored)
    //
    // NOTE: The target device does not have DisplayYesNo capabilities.
    //       For this reason, the passkey reply system is unused.
    case ESP_GAP_BLE_PASSKEY_REQ_EVT: /* passkey request event */
        ESP_LOGI(TAG, "ESP_GAP_BLE_PASSKEY_REQ_EVT");
        // esp_ble_passkey_reply(&profile_table[PROFILE_neil_ID].remote_bda,
        // true,
        //                       0x00);
        break;

    // --- On Out-of-band Pairing Request
    case ESP_GAP_BLE_OOB_REQ_EVT: {
        ESP_LOGI(TAG, "ESP_GAP_BLE_OOB_REQ_EVT");
        uint8_t tk[16] = {
            1}; // If you paired with OOB, both devices need to use the same tk
        esp_ble_oob_req_reply(param->ble_security.ble_req.bd_addr, tk,
                              sizeof(tk));
        break;
    }

    // --- On Local Identity-Root (ignored)
    case ESP_GAP_BLE_LOCAL_IR_EVT: /* BLE local IR event */
        ESP_LOGI(TAG, "ESP_GAP_BLE_LOCAL_IR_EVT");
        break;

    // --- On Local Encryption-Root (ignored)
    case ESP_GAP_BLE_LOCAL_ER_EVT: /* BLE local ER event */
        ESP_LOGI(TAG, "ESP_GAP_BLE_LOCAL_ER_EVT");
        break;

    // On Numeric Comparison Request (Compare pass-key on pairing)
    case ESP_GAP_BLE_NC_REQ_EVT:
        /* The app will receive this evt when the IO has DisplayYesNO capability
        and the peer device IO also has DisplayYesNo capability. show the
        passkey number to the user to confirm it with the number displayed by
        peer device. */
        esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, true);
        ESP_LOGI(TAG,
                 "ESP_GAP_BLE_NC_REQ_EVT, the passkey Notify number:%" PRIu32,
                 param->ble_security.key_notif.passkey);
        break;

    // --- On BLE Security Request
    case ESP_GAP_BLE_SEC_REQ_EVT:
        /* send the positive(true) security response to the peer device to
        accept the security request. If not accept the security request, should
        send the security response with negative(false) accept value*/
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
        break;

    // --- On Passkey Notification
    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT: /// the app will receive this evt when
                                        /// the IO  has Output capability and
                                        /// the peer device IO has Input
                                        /// capability.

        /// show the passkey number to the user to input it in the peer device.
        ESP_LOGI(TAG, "The passkey Notify number:%06" PRIu32,
                 param->ble_security.key_notif.passkey);
        break;

    // --- On BLE Key Event for Peer Device Keys
    case ESP_GAP_BLE_KEY_EVT:
        // shows the ble key info share with peer device to the user.
        ESP_LOGI(
            TAG, "key type = %s",
            neil_ble_gatts_util_esp_key_to_str(param->ble_security.ble_key.key_type));
        break;

    // --- On Authentication Done
    case ESP_GAP_BLE_AUTH_CMPL_EVT: {
        esp_bd_addr_t bd_addr;
        memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr,
               sizeof(esp_bd_addr_t));
        ESP_LOGI(TAG, "remote BD_ADDR: %08x%04x",
                 (bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) +
                     bd_addr[3],
                 (bd_addr[4] << 8) + bd_addr[5]);
        ESP_LOGI(TAG, "address type = %d",
                 param->ble_security.auth_cmpl.addr_type);
        ESP_LOGI(TAG, "pair status = %s",
                 param->ble_security.auth_cmpl.success ? "success" : "fail");
        if (!param->ble_security.auth_cmpl.success) {
            ESP_LOGI(TAG, "fail reason = 0x%x",
                     param->ble_security.auth_cmpl.fail_reason);
        } else {
            ESP_LOGI(TAG, "auth mode = %s",
                     neil_ble_gatts_util_esp_auth_req_to_str(
                         param->ble_security.auth_cmpl.auth_mode));
        }
        neil_ble_gatts_util_show_bonded_devices(TAG);
        break;
    }

    // --- On Bonded Device Removal
    case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT: {
        ESP_LOGD(TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT status = %d",
                 param->remove_bond_dev_cmpl.status);
        ESP_LOGI(TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV");
        ESP_LOGI(TAG, "-----ESP_GAP_BLE_REMOVE_BOND_DEV----");
        esp_log_buffer_hex(TAG, (void *)param->remove_bond_dev_cmpl.bd_addr,
                           sizeof(esp_bd_addr_t));
        ESP_LOGI(TAG, "------------------------------------");
        break;
    }

    // --- On Privacy Toggle
    case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
        if (param->local_privacy_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(TAG, "config local privacy failed, error status = %x",
                     param->local_privacy_cmpl.status);
            break;
        }

        esp_err_t ret = esp_ble_gap_config_adv_data(&gap_config.adv_data);
        if (ret) {
            ESP_LOGE(TAG, "config adv data failed, error code = %x", ret);
        } else {
            is_adv_config_done |= ADV_CONFIG_COMPLETED_FLAG;
        }

        ret = esp_ble_gap_config_adv_data(&gap_config.adv_ext_data);
        if (ret) {
            ESP_LOGE(TAG, "config adv ext data failed, error code = %x", ret);
        } else {
            is_adv_config_done |= SCAN_RSP_CONFIG_COMPLETED_FLAG;
        }

        break;

    // --- Unrecognized Event
    default:
        break;
    }
}

// HACK: Refactor into parameterized configuration boilerplate
//
// In it's current state, it is 1:1 with the GATT security example
//
void neil_ble_gatts_gap_configure_security() {
    /* set the security iocap & auth_req & key size & init key response key
     * parameters to the stack*/

    // HACK: Move out into parameter
    esp_ble_auth_req_t auth_req =
        ESP_LE_AUTH_REQ_SC_MITM_BOND; // bonding with peer device after
                                      // authentication

    // HACK: Move out into parameter
    esp_ble_io_cap_t iocap =
        ESP_IO_CAP_NONE; // set the IO capability to No output No input

    // HACK: Move out into parameter
    uint8_t key_size = 16; // the key size should be 7~16 bytes
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key  = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;

    // Passkey (Randomly Typed)
    //
    // HACK: Move out into parameter
    //
    uint32_t passkey = 0xFF09E48D;

    uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
    uint8_t oob_support = ESP_BLE_OOB_DISABLE;

    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey,
                                   sizeof(uint32_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req,
                                   sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap,
                                   sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size,
                                   sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH,
                                   &auth_option, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_OOB_SUPPORT, &oob_support,
                                   sizeof(uint8_t));
    /* If your BLE device acts as a Slave, the init_key means you hope which
    types of key of the master should distribute to you, and the response key
    means which key you can distribute to the master; If your BLE device acts as
    a master, the response key means you hope which types of key of the slave
    should distribute to you, and the init key means which key you can
    distribute to the slave. */
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key,
                                   sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key,
                                   sizeof(uint8_t));
}

static void adv_svc_uuid_merge(const neil_ble_gatts_cfg_dev_t *dev_cfg,
                               uint8_t *uuid) {

    ESP_LOGI(TAG, "Merging Service UUIDs for advertising");

    // For each service in the device config
    for (uint8_t svc_idx = 0; svc_idx < dev_cfg->svc_tab_len; svc_idx++) {
        uint16_t offset = ESP_UUID_LEN_128 * svc_idx;

        uint8_t *svc_id = (dev_cfg->svc_tab + svc_idx)->uuid;

        // Create a UUID to relate to the service
        memcpy(uuid + offset, svc_id, ESP_UUID_LEN_128);
    }
}
