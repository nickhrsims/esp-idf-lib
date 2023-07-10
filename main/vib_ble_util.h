#ifndef VIB_BLE_UTIL_H_
#define VIB_BLE_UTIL_H_

#include "esp_gap_ble_api.h"

char *vib_ble_util_esp_key_to_str(esp_ble_key_type_t key_type);
char *vib_ble_util_esp_auth_req_to_str(esp_ble_auth_req_t auth_req);
void vib_ble_util_show_bonded_devices(const char *const tag);

#endif // VIB_BLE_UTIL_H_
