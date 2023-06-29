/// main.c
///
/// @brief       Houses application entry-point.

#include "vib_audio.h"
#include "vib_memory.h"

void app_main(void) {
    vib_memory_init();
    vib_audio_start();
}
