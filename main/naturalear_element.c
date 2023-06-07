#include <string.h>
#include "esp_log.h"
#include "audio_error.h"
#include "audio_mem.h"
#include "audio_element.h"
#include "naturalear_element.h"
#include "audio_type_def.h"

static const char *TAG = "NATURALEAR";

// -------------------------------------------------------------
// Forward Declarations
// -------------------------------------------------------------
static esp_err_t open(audio_element_handle_t self);
static esp_err_t close(audio_element_handle_t self);
static esp_err_t destroy(audio_element_handle_t self);
static audio_element_err_t process(audio_element_handle_t self, char *input_buffer, int input_buffer_size);

// -------------------------------------------------------------
// Filter Data
// -------------------------------------------------------------
//
// NOTE: It appears that the only reason this is needed is because a buffer is
//       not prepared ahead of time
//       (due to configuration parameters of underlying audio element handle)
//
//       No filter data is needed if a buffer length is chosen.

// -------------------------------------------------------------
// Initilize
// -------------------------------------------------------------

audio_element_handle_t naturalear_audio_element_init(naturalear_audio_element_cfg_t *naturalear_audio_element_cfg)
{
    //
    // Check Configuration
    //
    if (naturalear_audio_element_cfg == NULL) {
        ESP_LOGE(TAG, "ne_config is NULL. (line %d)", __LINE__);
        return NULL;
    }

    //
    // Configure Base Audio Element
    //
    audio_element_cfg_t audio_element_cfg = DEFAULT_AUDIO_ELEMENT_CONFIG();

    audio_element_cfg.process = process;
    audio_element_cfg.open    = open;
    audio_element_cfg.close   = close;
    audio_element_cfg.destroy = destroy;

    audio_element_cfg.buffer_len   = 256;
    audio_element_cfg.tag          = "naturalear";


    // NOTE: These can probably be removed, as they aren't subject to current configuration.
    // (i.e. allow these to be populated by defaults factory macro)
    audio_element_cfg.task_stack   = naturalear_audio_element_cfg->task_stack_size;
    audio_element_cfg.task_prio    = naturalear_audio_element_cfg->task_priority;
    audio_element_cfg.task_core    = naturalear_audio_element_cfg->task_core;
    audio_element_cfg.task_core    = naturalear_audio_element_cfg->task_core;
    audio_element_cfg.out_rb_size  = naturalear_audio_element_cfg->output_ringbuffer_size;
    audio_element_cfg.stack_in_ext = naturalear_audio_element_cfg->attempt_external_stack_allocation;

    //
    // Build Base Audio Element
    //
    audio_element_handle_t audio_element_handle = audio_element_init(&audio_element_cfg);

    AUDIO_MEM_CHECK(TAG, audio_element_handle, {return NULL;});
    ESP_LOGI(TAG, "Initialized");

    return audio_element_handle;
}

// -------------------------------------------------------------
// Process
// -------------------------------------------------------------

/// Audio Process Callback
static audio_element_err_t process(audio_element_handle_t self, char *input_buffer, int input_buffer_length) {
    ESP_LOGI(TAG, "Start of Process Callback");
    //
    // Get input buffer + size
    //
    int read_size = audio_element_input(self, input_buffer, input_buffer_length);

    //
    // Default write buffer size
    //
    int write_size = 0;

    //
    // if there are samples to read
    //   redirect them to output
    //
    // else
    //   return 0
    //   considered an error by ADF
    //   see: `audio_element_err_t`
    //
    if (read_size > 0) {
        ESP_LOGI(TAG, "Processed Audio (did nothing)");
        write_size = audio_element_output(self, input_buffer, read_size);
    } else {
        ESP_LOGI(TAG, "Read Size is %d", read_size);
        write_size = read_size;
    }

    ESP_LOGI(TAG, "End of Process Callback");
    return write_size;
}


// -------------------------------------------------------------
// Open / Close / Destroy
// -------------------------------------------------------------

static esp_err_t open(audio_element_handle_t self) {
    ESP_LOGI(TAG, "Opened");
    return ESP_OK;
}

static esp_err_t close(audio_element_handle_t self) {
    ESP_LOGI(TAG, "Closed");
    return ESP_OK;
}

static esp_err_t destroy(audio_element_handle_t self) {
    ESP_LOGI(TAG, "Destroyed");
    return ESP_OK;
}
