/// vib_audio.h
///
/// @brief       Prototypes audio layer controls.

#ifndef VIB_AUDIO_H_
#define VIB_AUDIO_H_

/**
 * @brief       Schedule the audio processing pipeline.
 *
 *              Handles audio I2S audio IO and processing.
 */
void vib_audio_start(void);

/**
 * @brief       Set audio gain.
 */
void vib_audio_set_gain(float value);

#endif // VIB_AUDIO_H_
