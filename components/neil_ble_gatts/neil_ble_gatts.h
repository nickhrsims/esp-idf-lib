// SPDX-FileCopyrightText: 2023 Nicholas H.R. Sims <nickhrsims@gmail.com>
//
// SPDX-License-Identifier: Apache-2.0

/// neil_ble_gatts.h

#ifndef neil_ble_gatts_H_
#define neil_ble_gatts_H_

#include "neil_ble_gatts_cfg.h"

// -------------------------------------------------------------
// Prototypes
// -------------------------------------------------------------

/**
 * @brief       Start a new Bluetooth Low-Energy GATT Server.
 *
 *              (Do not start more than one server)
 */
void neil_ble_gatts_start(const neil_ble_gatts_cfg_dev_t *dev_cfg);

#endif // neil_ble_gatts_H_
