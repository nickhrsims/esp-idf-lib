# SPDX-FileCopyrightText: 2023 Nicholas H.R. Sims <nickhrsims@gmail.com>
#
# SPDX-License-Identifier: Apache-2.0

idf_component_register(
  SRCS
    "neil_ble_gatts.h"
    "neil_ble_gatts_attr_db.h"
    "neil_ble_gatts_gap.c"
    "neil_ble_gatts_util.c"
    "neil_ble_gatts.c"
    "neil_ble_gatts_attr_db.c"
    "neil_ble_gatts_cfg.h"
    "neil_ble_gatts_gap.h"
    "neil_ble_gatts_util.h"

    INCLUDE_DIRS
      .

    REQUIRES
      bt
  )
