/// vib_audio.c
///
/// @brief       Orchestrates audio layer components.

#include "esp_task.h"
#include "freertos/task.h"

#include "vib_audio.h"
#include "vib_audio_params.h"
#include "vib_audio_pipeline.h"

#define APP_AUDIO_PIPELINE_TASK_STACK_SIZE  (1024 * 8)
#define APP_AUDIO_PIPELINE_TASK_PARAM_COUNT 0
#define APP_AUDIO_PIPELINE_TASK_PRIORITY    (ESP_TASK_MAIN_PRIO + 1)
#define APP_AUDIO_PIPELINE_TASK_CORE        ESP_TASK_MAIN_CORE
static TaskHandle_t vib_audio_pipeline_task_handle;

/**
 * @brief       Schedule the audio processing pipeline.
 *
 *              Handles audio I2S audio IO and processing.
 */
void vib_audio_start(void) {
    xTaskCreatePinnedToCore(
        vib_audio_pipeline_task, "VIB Audio Pipeline Task",
        APP_AUDIO_PIPELINE_TASK_STACK_SIZE, APP_AUDIO_PIPELINE_TASK_PARAM_COUNT,
        APP_AUDIO_PIPELINE_TASK_PRIORITY, &vib_audio_pipeline_task_handle,
        APP_AUDIO_PIPELINE_TASK_CORE);
}

void vib_audio_set_gain(float value) { vib_audio_param_gain = value; }
