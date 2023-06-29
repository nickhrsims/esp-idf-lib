#include "esp_err.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "vib_memory.h"

esp_err_t vib_memory_init() {
    esp_err_t error_code;

    error_code = nvs_flash_init();
    if (error_code == ESP_ERR_NVS_NO_FREE_PAGES ||
        error_code == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        error_code = nvs_flash_init();
    }
    ESP_ERROR_CHECK(error_code);

    return error_code;
}
