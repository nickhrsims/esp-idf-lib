#ifndef VIB_BLE_GAP_H_
#define VIB_BLE_GAP_H_

#include "esp_gap_ble_api.h"

#include "vib_ble_cfg.h"

void vib_ble_gap_init(const vib_ble_cfg_dev_t *dev_cfg);
void vib_ble_gap_advertise();
void vib_ble_gap_event_handler(esp_gap_ble_cb_event_t event,
                               esp_ble_gap_cb_param_t *param);
void vib_ble_gap_configure_security();
#endif // VIB_BLE_GAP_H_
