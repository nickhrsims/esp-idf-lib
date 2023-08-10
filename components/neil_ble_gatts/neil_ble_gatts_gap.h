// SPDX-FileCopyrightText: 2023 Nicholas H.R. Sims <nickhrsims@gmail.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef neil_ble_gatts_GAP_H_
#define neil_ble_gatts_GAP_H_

#include "esp_gap_ble_api.h"

#include "neil_ble_gatts_cfg.h"

void neil_ble_gatts_gap_init(const neil_ble_gatts_cfg_dev_t *dev_cfg);
void neil_ble_gatts_gap_advertise();
void neil_ble_gatts_gap_event_handler(esp_gap_ble_cb_event_t event,
                               esp_ble_gap_cb_param_t *param);
void neil_ble_gatts_gap_configure_security();
#endif // neil_ble_gatts_GAP_H_
