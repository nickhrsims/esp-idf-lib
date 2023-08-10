// SPDX-FileCopyrightText: 2023 Nicholas H.R. Sims <nickhrsims@gmail.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef neil_ble_gatts_UTIL_H_
#define neil_ble_gatts_UTIL_H_

#include "esp_gap_ble_api.h"

char *neil_ble_gatts_util_esp_key_to_str(esp_ble_key_type_t key_type);
char *neil_ble_gatts_util_esp_auth_req_to_str(esp_ble_auth_req_t auth_req);
void neil_ble_gatts_util_show_bonded_devices(const char *const tag);

#endif // neil_ble_gatts_UTIL_H_
