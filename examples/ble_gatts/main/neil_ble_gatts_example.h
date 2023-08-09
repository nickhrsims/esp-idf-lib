#pragma once

// -------------------------------------------------------------
// Example UUID System
// -------------------------------------------------------------
//
// Base UUID Auxiliary Macro
//
// Wraps a 16-bit attribute UUID within a base UUID.
//
// NOTE: This is not required when using SIG-defined 16-bit UUIDs
//
// Format:
//     XX - 8-bit Service Index
//     YY - 8-bit Characteristic Index
//     00000000-XXYY-0000-0000-000000000000
//
//     XX:    SERVICE       @ 11
//     YY: CHARACTERISTIC   @ 10
//
// Parameters:
//     (uint8_t)    service_index     - Internal Service ID.
//
//     (uint8_t) characteristic_index - Internal Characteristic ID.
//                                NOTE: Must be `0` when declaring a service.
//
// Note:
//     Parameters (atow) are not used for internal purposes, any range of
//     128-bit UUIDs will suffice that do not conflict the with the Bluetooth
//     Low-Energy spec 128-bit UUID range (or vendor-specific ones).
//
//     i.e. Official Spec: 0000NNNN-0000-1000-8000-00805F9B34FB
//
// Usage:
//     APP_BLE_UUID_128(service_index, characteristic_index)
//
// @see https://www.uuidgenerator.net/

/// Wrap 16-bit UUID in 128-bit base
#define APP_BLE_UUID_128(XX, YY)                                                       \
    {                                                                                  \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, YY, XX, 0x00,      \
            0x00, 0x00, 0x00                                                           \
    }

/// Get service index (by-convention) from UUID.
#define APP_BLE_UUID_128_GET_SVC_INDEX(uuid128) uuid128[11]

/// Get characteristic index (by-convention) from UUID.
#define APP_BLE_UUID_128_GET_CHR_INDEX(uuid128) uuid128[10]
