#ifndef VIB_BLE_CONFIG_H_
#define VIB_BLE_CONFIG_H_

#include <stddef.h>
#include <stdint.h>

// -------------------------------------------------------------
// Freudensong Attribute Type UUIDs
// -------------------------------------------------------------
//
// Base UUID Auxiliary Macro
//
// Wraps an 8-bit attribute UUID within a base UUID.
//
// Format:
//     XX - 8-bit Service Index
//     YY - 8-bit Characteristic Index
//     C2D5B9D6-XXYY-452E-84D1-0A0C537A36D7
//
//     XX: INDEX 11
//     YY: INDEX 10
//
// Usage:
//     VIB_BLE_UUID_128(service_index, characteristic_index)
//
// @see https://www.uuidgenerator.net/
#define VIB_BLE_UUID_128(XX, YY)                                               \
    /* NOTE: Do not remove the type-hint from this struct initializer */       \
    (uint8_t[16]) {                                                            \
        0xD7, 0x36, 0x7A, 0x53, 0x0C, 0x0A, 0xD1, 0x84, 0x2E, 0x45, YY, XX,    \
            0xD6, 0xB9, 0xD5, 0xC2                                             \
    }

// -------------------------------------------------------------
// Device Configuration Structures
// -------------------------------------------------------------

typedef struct {
    uint8_t on_read;
    uint8_t on_write;
    uint16_t max_length;
    uint16_t initial_length;
    uint8_t *initial_value;
} vib_ble_characteristic_config_t;

typedef struct {
    uint8_t characteristic_count;
    vib_ble_characteristic_config_t *characteristics;
} vib_ble_service_config_t;

typedef struct {
    uint8_t device_name_length;
    char *device_name;
    uint8_t manufacturer_name_length;
    char *manufacturer_name;
    uint8_t service_count;
    vib_ble_service_config_t *services;
} vib_ble_device_config_t;

// -------------------------------------------------------------
// Device Configuration API
// -------------------------------------------------------------

vib_ble_device_config_t *vib_ble_device_config_get();
void vib_ble_device_config_set(vib_ble_device_config_t *device_config);

#endif // VIB_BLE_CONFIG_H_
