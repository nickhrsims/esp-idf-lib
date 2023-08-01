/// vib_audio_element.c
///
/// @author     Nicholas H.R. Sims
///
/// @brief      Custom Audio Element based on the Equilizer Audio Element
///             provided with the ESP ADF Audio Codec Library
///
///             Much of this file's content was taken right out of the original
///             element. New components can be created from this foundation
///             (or better, from the Codec library examples)
///
/// @see    https://github.com/espressif/esp-adf.git
/// @see    esp-adf/components/esp-adf-libs/esp_codec/include/codec/equilizer.h
/// @see    esp-adf/examples/audio_processing/pipeline_equilizer/README.md

#include "audio_element.h"
#include "audio_error.h"
#include "esp_log.h"
#include <string.h>

#include "vib_audio_element.h"
#include "vib_audio_params.h"

// -------------------------------------------------------------
// Auxiliary
// -------------------------------------------------------------
// Combine bytes to word
#define WORD(byte0, byte1) (((int16_t)(byte1 << 8) | byte0))
// Low byte of word
#define BYTE0(word) ((char)word)
// High byte of word
#define BYTE1(word) ((char)((int16_t)word >> 8))
// Cast word to float
#define FLOAT(word) ((float)word)
// Cast float to word
#define FWORD(float_) ((int16_t)float_)

// -------------------------------------------------------------
// Constants
// -------------------------------------------------------------
static const char *TAG = "VIB";

// -------------------------------------------------------------
// Types
// -------------------------------------------------------------
typedef int16_t audio_sample_t;
typedef int8_t audio_sample_byte_t;

// -------------------------------------------------------------
// Forward Declarations
// -------------------------------------------------------------
static esp_err_t open(audio_element_handle_t self);
static esp_err_t close(audio_element_handle_t self);
static esp_err_t destroy(audio_element_handle_t self);
static audio_element_err_t process(audio_element_handle_t self,
                                   char *input_buffer, int input_buffer_size);

// -------------------------------------------------------------
// Initilize
// -------------------------------------------------------------
audio_element_handle_t
vib_audio_element_init(vib_audio_element_cfg_t *vib_audio_element_cfg) {
    //
    // Check Configuration
    //

    // NOTE: Not part of the ADF!
    if (vib_audio_element_cfg == NULL) {
        ESP_LOGE(TAG, "ne_config is NULL. (line %d)", __LINE__);
        return NULL;
    }

    //
    // Configure Base Audio Element
    //
    audio_element_cfg_t audio_element_cfg = DEFAULT_AUDIO_ELEMENT_CONFIG();

    // Callback Registration
    audio_element_cfg.process = process;
    audio_element_cfg.open    = open;
    audio_element_cfg.close   = close;
    audio_element_cfg.destroy = destroy;

    // Input buffer size (in bytes)
    audio_element_cfg.buffer_len =
        vib_audio_element_cfg->output_ringbuffer_size;
    audio_element_cfg.tag = "vib";

    // NOTE: These can probably be removed, as they aren't subject to
    // current configuration. (i.e. allow these to be populated by defaults
    // factory macro) -nick
    audio_element_cfg.task_stack = vib_audio_element_cfg->task_stack_size;
    audio_element_cfg.task_prio  = vib_audio_element_cfg->task_priority;
    audio_element_cfg.task_core  = vib_audio_element_cfg->task_core;
    audio_element_cfg.task_core  = vib_audio_element_cfg->task_core;
    audio_element_cfg.out_rb_size =
        vib_audio_element_cfg->output_ringbuffer_size;
    audio_element_cfg.stack_in_ext =
        vib_audio_element_cfg->attempt_external_stack_allocation;

    //
    // Build Base Audio Element
    //
    audio_element_handle_t audio_element_handle =
        audio_element_init(&audio_element_cfg);

    AUDIO_MEM_CHECK(TAG, audio_element_handle, { return NULL; });
    ESP_LOGI(TAG, "Initialized");

    return audio_element_handle;
}

// -------------------------------------------------------------
// Process
// -------------------------------------------------------------

/// Audio Process Callback
static audio_element_err_t process(audio_element_handle_t self,
                                   char *input_buffer,
                                   int input_buffer_length) {
    //
    // Get input buffer + size
    //
    int read_size =
        audio_element_input(self, input_buffer, input_buffer_length);

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

        for (char *spool = input_buffer;
             spool != input_buffer + input_buffer_length; spool += 2) {

            audio_sample_t sample =
                WORD(*spool, *(spool + 1)) * vib_audio_param_gain;

            *spool       = BYTE0(sample);
            *(spool + 1) = BYTE1(sample);
        }

        write_size = audio_element_output(self, input_buffer, read_size);
    } else {
        ESP_LOGW(TAG, "Read Size is %d", read_size);
        write_size = read_size;
    }

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
