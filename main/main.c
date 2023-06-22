#include "esp_task.h"
#include "freertos/task.h"

#include "audio/pipeline.h"

#define APP_AUDIO_PIPELINE_TASK_STACK_SIZE 1024
#define APP_AUDIO_PIPELINE_TASK_PARAM_COUNT 0
#define APP_AUDIO_PIPELINE_TASK_PRIORITY (ESP_TASK_MAIN_PRIO + 1)
#define APP_AUDIO_PIPELINE_TASK_CORE ESP_TASK_MAIN_CORE
static TaskHandle_t vib_audio_pipeline_task_handle;

static const char *TAG = "APP";

/**
 * @brief       Schedule the audio processing pipeline.
 *
 *              Handles audio I2S audio IO and processing.
 */
static void schedule_audio(void) {
    xTaskCreatePinnedToCore(
        vib_audio_pipeline_task, "VIB Audio Pipeline Task",
        APP_AUDIO_PIPELINE_TASK_STACK_SIZE, APP_AUDIO_PIPELINE_TASK_PARAM_COUNT,
        APP_AUDIO_PIPELINE_TASK_PRIORITY, &vib_audio_pipeline_task_handle,
        APP_AUDIO_PIPELINE_TASK_CORE);
}

/**
 * @brief       Schedule the bluetooth low-energy GATT server.
 *
 *              Handles communication with bluetooth connected application.
 */
static void schedule_gatt_server(void) {}

void app_main(void) { schedule_audio(); }
