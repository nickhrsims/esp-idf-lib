// naturalear_element.h
//
// @author     Nicholas H.R. Sims
//
// @brief      Custom Audio Element based on the Equilizer Audio Element
//             provided with the ESP ADF Audio Codec Library
//
//             Much of this file's content was taken right out of the original
//             element. New components can be created from this foundation
//             (or better, from the Codec library examples)
//
// @see        https://github.com/espressif/esp-adf.git
// @see        esp-adf/components/esp-adf-libs/esp_codec/include/codec/equilizer.h
// @see        esp-adf/examples/audio_processing/pipeline_equilizer/README.md
//
#ifndef NATURALEAR_ELEMENT_H_
#define NATURALEAR_ELEMENT_H_

#include "esp_err.h"
#include "audio_element.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================================================================== *\
  / - - - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - \
 <*                                  Begin                                   *>
  \ - - - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - /
\* ========================================================================== */

// -------------------------------------------------------------
// Configuration
// -------------------------------------------------------------
/**
 * @brief      Element Configuration
 * NOTE: contents not specified adf api
 */
typedef struct naturalear_audio_element_cfg {
    int samplerate;                          /* Audio sample rate (in Hz)*/
    int channels;                            /* Number of audio channels (Mono=1, Dual=2) */
    int output_ringbuffer_size;              /* Size of output ring buffer */
    int task_stack_size;                     /* Task stack size */
    int task_core;                           /* Task running in core...*/
    int task_priority;                       /* Task priority */
    bool attempt_external_stack_allocation;  /* Try to allocate stack in external memory */
} naturalear_audio_element_cfg_t;

#define NATURALEAR_RINGBUFFER_SIZE       (8 * 1024)
#define NATURALEAR_TASK_STACK_SIZE       (4 * 1024)
#define NATURALEAR_TASK_CORE             (0)
#define NATURALEAR_TASK_PRIORITY         (5)

#define DEFAULT_NATURALEAR_CONFIG() {                                  \
    .samplerate                        = 48000,                        \
    .channels                          = 1,                            \
    .output_ringbuffer_size            = NATURALEAR_RINGBUFFER_SIZE,   \
    .task_stack_size                   = NATURALEAR_TASK_STACK_SIZE,   \
    .task_core                         = NATURALEAR_TASK_CORE,         \
    .task_priority                     = NATURALEAR_TASK_PRIORITY,     \
    .attempt_external_stack_allocation = true,                         \
}



// -------------------------------------------------------------
// External Functions
// -------------------------------------------------------------

audio_element_handle_t naturalear_audio_element_init(naturalear_audio_element_cfg_t *config);


/* ========================================================================== *\
  / - - - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - \
 <*                                  End                                     *>
  \ - - - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - /
\* ========================================================================== */

#ifdef __cplusplus
}
#endif

#endif // NATURALEAR_ELEMENT_H_
