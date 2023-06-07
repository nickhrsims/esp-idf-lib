#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "naturalear_element.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "i2s_stream.h"
#include "esp_peripherals.h"
#include "board.h"

static const char *TAG = "APP";

// app_main:
//     setup and then start pipeline
//     then respond to events; stop event tears the system down

void app_main(void)
{
  // ---------------------------------
  // Logging
  // ---------------------------------

  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set(TAG, ESP_LOG_DEBUG);

  // ---------------------------------
  // Initialization
  // ---------------------------------

  audio_pipeline_handle_t pipeline;
  audio_element_handle_t i2s_stream_writer, i2s_stream_reader, naturalear;

  ESP_LOGI(TAG, "[ 1 ] Start codec chip");

  // NOTE: Unused
  audio_board_handle_t board_handle __attribute__((unused)) = audio_board_init();

  audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_LINE_IN,
                       AUDIO_HAL_CTRL_START);

  ESP_LOGI(TAG, "[ 2 ] Create audio pipeline for playback");
  audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
  pipeline = audio_pipeline_init(&pipeline_cfg);

  ESP_LOGI(TAG, "[3.1] Create i2s stream to write data to codec chip");
  i2s_stream_cfg_t i2s_cfg_write = I2S_STREAM_CFG_DEFAULT();
  i2s_cfg_write.type = AUDIO_STREAM_WRITER;
  i2s_stream_writer = i2s_stream_init(&i2s_cfg_write);

  ESP_LOGI(TAG, "[3.2] Create i2s stream to read data from codec chip");
  i2s_stream_cfg_t i2s_cfg_read = I2S_STREAM_CFG_DEFAULT();
  i2s_cfg_read.type = AUDIO_STREAM_READER;
  i2s_stream_reader = i2s_stream_init(&i2s_cfg_read);

  ESP_LOGI(TAG, "[3.3] Create ne filter to process stream");
  naturalear_audio_element_cfg_t naturalear_cfg = DEFAULT_NATURALEAR_CONFIG();
  naturalear = naturalear_audio_element_init(&naturalear_cfg);


  // ---------------------------------
  // Pipeline Wiring
  // ---------------------------------

  /* order doesn't matter when registering */
  ESP_LOGI(TAG, "[3.3] Register all elements to audio pipeline");
  audio_pipeline_register(pipeline, i2s_stream_reader, "i2s_read");
  audio_pipeline_register(pipeline, i2s_stream_writer, "i2s_write");
  audio_pipeline_register(pipeline, naturalear, "filter");

  ESP_LOGI(
      TAG,
      "[3.4] Link it together "
      "[codec_chip]-->i2s_stream_reader-->naturalear-->i2s_stream_writer-->[codec_chip]");
  /* specify sequence by names */
  const char* link_tags[3] = { "i2s_read", "filter", "i2s_write" };
  /* register sequence */
  audio_pipeline_link(pipeline, &link_tags[0], 3);


  // ---------------------------------
  // Event System Initialization
  // ---------------------------------

  ESP_LOGI(TAG, "[ 4 ] Set up  event listener");
  audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
  audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

  ESP_LOGI(TAG, "[4.1] Listening event from all elements of pipeline");
  audio_pipeline_set_listener(pipeline, evt);

  ESP_LOGI(TAG, "[ 5 ] Start audio_pipeline");
  audio_pipeline_run(pipeline);

  // ---------------------------------
  // Event Handling
  // ---------------------------------

  ESP_LOGI(TAG, "[ 6 ] Listen for all pipeline events");
  while (1) {
    audio_event_iface_msg_t msg;
    /* blocking listen: */
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

  ESP_LOGI(TAG, "[ 7 ] Stop audio_pipeline");
  audio_pipeline_stop(pipeline);
  audio_pipeline_wait_for_stop(pipeline);
  audio_pipeline_terminate(pipeline);

  audio_pipeline_unregister(pipeline, i2s_stream_reader);
  audio_pipeline_unregister(pipeline, naturalear);
  audio_pipeline_unregister(pipeline, i2s_stream_writer);

  /* Terminate the pipeline before removing the listener */
  audio_pipeline_remove_listener(pipeline);

  /* Make sure audio_pipeline_remove_listener &
   * audio_event_iface_remove_listener are called before destroying event_iface
   */
  audio_event_iface_destroy(evt);

  /* Release all resources */
  audio_pipeline_deinit(pipeline);
  audio_element_deinit(i2s_stream_reader);
  audio_element_deinit(naturalear);
  audio_element_deinit(i2s_stream_writer);
}
