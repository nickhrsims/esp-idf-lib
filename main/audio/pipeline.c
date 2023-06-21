// pipeline.c
//
// @author     Nicholas H.R. Sims
//
// @brief      Audio pipeline configuration based on the Equilizer example
//             provided with the ESP ADF.
//
//             Much of this file's content was taken right out of the original
//             module.
//
// @see        https://github.com/espressif/esp-adf.git
// @see        esp-adf/examples/audio_processing/pipeline_equilizer/README.md
//

#include "audio_common.h"
#include "audio_element.h"
#include "audio_event_iface.h"
#include "audio_pipeline.h"
#include "board.h"
#include "esp_log.h"
#include "i2s_stream.h"
#include <string.h>

#include "pipeline.h"
#include "vib_audio_element.h"

// -------------------------------------------------------------
// Module-Static Data
// -------------------------------------------------------------

/// Logging tag
static const char *TAG = "AUDIO PIPELINE";

/// Pipeline Handle
static audio_pipeline_handle_t pipeline;

/// Stream Handles
static audio_element_handle_t i2s_stream_reader;
static audio_element_handle_t i2s_stream_writer;

/// VIB Audio Element
static audio_element_handle_t vib;

/// Board Data Handle
static audio_board_handle_t __attribute__((unused)) board_handle;

/// Event Bus Interface Handle
static audio_event_iface_handle_t evt;

// -------------------------------------------------------------
// Forwarded Declarations
// -------------------------------------------------------------

static void initialize(void);
static void listen(void);
static void terminate(void);

// -------------------------------------------------------------
// Entry-point (Managed Procedures)
// -------------------------------------------------------------

/// Primary pipeline task to be called by a scheduler.
void app_audio_pipeline_task(void *_) {
    // Setup/Start the pipeline.
    initialize();

    // Block-and-loop while waiting for events.
    // Audio elements are pushed scheduled in a dedicated audio thread.
    // Returns when a stop event is received.
    listen();

    // Stop/Teardown the pipeline.
    terminate();
}

// -------------------------------------------------------------
// Initialization
// -------------------------------------------------------------

/**
 * @brief      Setup and then start the pipline.
 *
 *             Project entry point (main).
 *             Create, and confgure pipeline, i2s_{in,out}, and vib.
 *             Wire pipeline:
 *
 *                 [i2s_in] ---> [vib] ---> [i2s_out]
 */
static void initialize(void) {
    // ---------------------------------
    // Logging
    // ---------------------------------

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    // ---------------------------------
    // Initialization
    // ---------------------------------

    ESP_LOGI(TAG, "[ 1 ] Start codec chip");

    // --- Initialize the ESP audio codec chip.
    // NOTE: Board handle is not used by the pipeline
    //       (or anywhere else, for that matter)
    board_handle = audio_board_init();

    // --- Configure the codec hardware abstraction layer.
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH,
                         AUDIO_HAL_CTRL_START);

    // --- Initialize the pipeline
    ESP_LOGI(TAG, "[ 2 ] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);

    // --- Initialize the read/write i2s stream controllers.
    ESP_LOGI(TAG, "[3.1] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg_write = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg_write.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg_write);

    ESP_LOGI(TAG, "[3.2] Create i2s stream to read data from codec chip");
    i2s_stream_cfg_t i2s_cfg_read = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg_read.type = AUDIO_STREAM_READER;
    i2s_stream_reader = i2s_stream_init(&i2s_cfg_read);

    // --- Initialize VIB audio stream processor.
    ESP_LOGI(TAG, "[3.3] Create vib filter to process stream");
    vib_audio_element_cfg_t vib_cfg = DEFAULT_VIB_CONFIG();
    vib = vib_audio_element_initialize(&vib_cfg);

    // ---------------------------------
    // Pipeline Wiring
    // ---------------------------------

    ESP_LOGI(TAG, "[3.3] Register all elements to audio pipeline");

    // NOTE: Registration is order independent.
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s_read");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s_write");
    audio_pipeline_register(pipeline, vib, "vib");

    ESP_LOGI(TAG,
             "[3.4] Link it together "
             "[codec_chip] -> i2s_stream_reader -> vib -> i2s_stream_writer "
             "-> [codec_chip]");

    // NOTE: Specify ordered sequence of tags.
    const char *link_tags[3] = {"i2s_read", "vib", "i2s_write"};
    //       Connect elements in the pipeline; in order of passed sequence.
    audio_pipeline_link(pipeline, &link_tags[0], 3);

    // ---------------------------------
    // Event System Initialization
    // ---------------------------------

    // NOTE: Event bus is for communication between audio streams
    //       and processing elements. Do not abuse to communicate with
    //       objects/entities outside of the pipeline.

    ESP_LOGI(TAG, "[ 4 ] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[4.1] Listening event from all elements of pipeline");
    audio_pipeline_set_listener(pipeline, evt);

    ESP_LOGI(TAG, "[ 5 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);
}

// -------------------------------------------------------------
// Event Handling
// -------------------------------------------------------------

/**
 * @brief       Block-and-loop listen for pipeline events.
 *             Respond to events
 *                 _stop_ event breaks loop
 */
static void listen(void) {
    // ---------------------------------
    // Event Handling
    // ---------------------------------

    ESP_LOGI(TAG, "[ 6 ] Listen for all pipeline events");
    while (1) {
        audio_event_iface_msg_t msg;

        // NOTE: Blocking call.
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }

        /* Stop when the last pipeline element (i2s_stream_writer in this case)
         * receives stop event */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT &&
            msg.source == (void *)i2s_stream_writer &&
            msg.cmd == AEL_MSG_CMD_REPORT_STATUS &&
            (((int)msg.data == AEL_STATUS_STATE_STOPPED) ||   // NOLINT
             ((int)msg.data == AEL_STATUS_STATE_FINISHED))) { // NOLINT
            ESP_LOGW(TAG, "[ * ] Stop event received");
            break;
        }
    }
}

// -------------------------------------------------------------
// Clean & Free
// -------------------------------------------------------------

/**
 * @brief       Terminate the pipeline and clean-up memory.
 *
 */
static void terminate(void) {
    ESP_LOGI(TAG, "[ 7 ] Stop audio_pipeline");
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);

    audio_pipeline_unregister(pipeline, i2s_stream_reader);
    audio_pipeline_unregister(pipeline, vib);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    /* Make sure audio_pipeline_remove_listener &
     * audio_event_iface_remove_listener are called before destroying
     * event_iface
     */
    audio_event_iface_destroy(evt);

    /* Release all resources */
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(i2s_stream_reader);
    audio_element_deinit(vib);
    audio_element_deinit(i2s_stream_writer);
}
