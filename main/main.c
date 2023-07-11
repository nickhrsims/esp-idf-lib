/// main.c
///
/// @brief       Houses application entry-point.

#include <string.h>

#include "vib_audio.h"
#include "vib_audio_params.h"
#include "vib_ble_cfg.h"
#include "vib_ble_gatt_server.h"
#include "vib_memory.h"

#define BLE_DEVICE_NAME "VIB"
#define BLE_MFR_NAME    "FREUDENSONG"

typedef union {
    float value;
    uint8_t raw[4];
} audio_param_dto_t;

void read_gain(uint8_t *buffer) {
    audio_param_dto_t gain_dto = {.value = vib_audio_param_gain};
    memcpy(buffer, gain_dto.raw, 4);
}

void write_gain(uint8_t *data, uint16_t len) {
    audio_param_dto_t gain_dto;
    memcpy(gain_dto.raw, data, len);
    vib_audio_set_gain(gain_dto.value);
}

static vib_ble_cfg_dev_t bluetooth_config = {

    .name_len = sizeof(BLE_DEVICE_NAME),
    .name     = BLE_DEVICE_NAME,

    .mfr_len = sizeof(BLE_MFR_NAME),
    .mfr     = BLE_MFR_NAME,

    .svc_tab_len = 1,
    .svc_tab =

        (vib_ble_cfg_svc_t[]){

            // --- Control Service ---
            {
                .uuid = VIB_BLE_UUID_128(0, 0),

                .chr_tab_len = 1,
                .chr_tab =
                    (vib_ble_cfg_chr_t[]){

                        // --- Gain ---
                        {
                            .uuid = VIB_BLE_UUID_128(0, 1),

                            .size = sizeof(float),

                            .on_read  = read_gain,
                            .on_write = write_gain,
                        },
                    },
            },
        },

};

void app_main(void) {
    vib_memory_init();
    vib_ble_gatt_server_start(&bluetooth_config);
    vib_audio_start();
}
