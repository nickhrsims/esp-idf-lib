/// vib_audio_element.h
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

#ifndef VIB_AUDIO_ELEMENT_H_
#define VIB_AUDIO_ELEMENT_H_

#include "audio_element.h"

#ifdef __cplusplus
extern "C" {
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
 *
 * NOTE: Contents not specified by ADF API.
 */
typedef struct vib_audio_element_cfg {
    int samplerate;             /* Audio sample rate (in Hz)*/
    int channels;               /* Number of audio channels (Mono=1, Dual=2) */
    int output_ringbuffer_size; /* Size of output ring buffer */
    int task_stack_size;        /* Task stack size */
    int task_core;              /* Task running in core...*/
    int task_priority;          /* Task priority */
    bool attempt_external_stack_allocation; /* Try to allocate stack in external
                                               memory */
} vib_audio_element_cfg_t;

#define VIB_RINGBUFFER_SIZE (8 * 1024)
#define VIB_TASK_STACK_SIZE (4 * 1024)
#define VIB_TASK_CORE (0)
#define VIB_TASK_PRIORITY (5)

#define DEFAULT_VIB_CONFIG()                                                   \
    {                                                                          \
        .samplerate = 44100, .channels = 1,                                    \
        .output_ringbuffer_size = VIB_RINGBUFFER_SIZE,                         \
        .task_stack_size = VIB_TASK_STACK_SIZE, .task_core = VIB_TASK_CORE,    \
        .task_priority = VIB_TASK_PRIORITY,                                    \
        .attempt_external_stack_allocation = true,                             \
    }

// -------------------------------------------------------------
// External Functions
// -------------------------------------------------------------

audio_element_handle_t vib_audio_element_init(vib_audio_element_cfg_t *config);

/* ========================================================================== *\
  / - - - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - \
 <*                                  End                                     *>
  \ - - - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - /
\* ========================================================================== */

#ifdef __cplusplus
}
#endif

#endif // VIB_AUDIO_ELEMENT_H_
