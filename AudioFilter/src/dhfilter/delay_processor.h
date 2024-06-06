#ifndef DELAY_PROCESSOR_H_
#define DELAY_PROCESSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FRAME_LENGTH 1024

// Define a type alias for the Delay Processor handle, which is a void pointer.
typedef void* DelayProcessorHandle;

// Create an instance of the delay processor and allocate memory for internal frequency division.
DelayProcessorHandle create_delay_processor(void);

// Initialize the delay processor algorithm by setting the sample rate, number of channels, and individual channel delays in milliseconds.
int initialize_delay_processor(DelayProcessorHandle handle,
                               unsigned int sample_rate,
                               unsigned int num_channels,
                               const float* channel_delays_ms);

// Process audio data through the delay processor.
int process_audio(DelayProcessorHandle handle,
                  short (*input_buffer)[FRAME_LENGTH],
                  short (*output_buffer)[FRAME_LENGTH],
                  unsigned int num_frames);

// Destroy the delay processor instance and release associated resources.
void destroy_delay_processor(DelayProcessorHandle handle);

#ifdef __cplusplus
}
#endif

#endif // DELAY_PROCESSOR_H_
