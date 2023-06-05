#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "i2s_stream.h"
#include "esp_peripherals.h"
#include "board.h"

static const char *TAG = "NATURALEAR_PIPELINE";

void app_main(void)
{
    ESP_LOGI(TAG, "[0] Empty Implementation");
}
