#ifndef DELAY_PROCESSOR_H_
#define DELAY_PROCESSOR_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "FilterTypes.h"
#define FRAME_LENGTH 1024

// Define a type alias for the Delay Processor handle, which is a void pointer.
typedef MVoid* DelayProcessorHandle;

// Create an instance of the delay processor and allocate memory for internal frequency division.
DelayProcessorHandle create_delay_processor(MVoid);

// Initialize the delay processor algorithm by setting the sample rate, number of channels, and individual channel delays in milliseconds.
MInt32 initialize_delay_processor(DelayProcessorHandle handle,
                                  MUInt32 sample_rate,
                                  MUInt32 num_channels,
                                  const MFloat* channel_delays_ms);

// Process audio data through the delay processor.
MInt32 process_audio(DelayProcessorHandle handle,
                     MInt16 (*input_buffer)[FRAME_LENGTH],
                     MInt16 (*output_buffer)[FRAME_LENGTH],
                     MUInt32 num_frames);

// Destroy the delay processor instance and release associated resources.
MVoid destroy_delay_processor(DelayProcessorHandle handle);

#ifdef __cplusplus
}
#endif

#endif // DELAY_PROCESSOR_H_
