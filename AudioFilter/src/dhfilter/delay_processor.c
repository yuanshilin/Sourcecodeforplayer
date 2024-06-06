#include "delay_processor.h"
#include <stdlib.h>
#include <string.h>

#define MAX_CHANNEL 24
#define MAX_DELAY 200.0f

// Internal data structure encapsulating information related to the delay processor.
struct DelayProcessor {
    unsigned int sample_rate;
    unsigned int num_channels;    
    int delay[MAX_CHANNEL];
    short* delay_lines[MAX_CHANNEL]; 
};

// Create an algorithm handle and allocate frequency division memory.
DelayProcessorHandle create_delay_processor(void) {
    struct DelayProcessor* dp = (struct DelayProcessor*) malloc(sizeof(struct DelayProcessor));
    if (dp == NULL) {
        return NULL;
    }

    dp->sample_rate = 0;
    dp->num_channels = 0;      
    for(int i = 0; i < MAX_CHANNEL; i++) {
        dp->delay[i] = 0;
        dp->delay_lines[i] = NULL;        
    }

    return (DelayProcessorHandle) dp;
}

// Initialize the algorithm by setting the sample rate, number of channels, and individual channel delays in milliseconds.
int initialize_delay_processor(DelayProcessorHandle handle,
                               unsigned int sample_rate,
                               unsigned int num_channels,
                               const float* channel_delays_ms) {
    struct DelayProcessor* dp = (struct DelayProcessor*) handle;
    if (dp == NULL || num_channels > MAX_CHANNEL) {
        return -1; // Invalid parameters or number of channels exceeds limit.
    }

    if (sample_rate != 8000 
        && sample_rate != 16000 
        && sample_rate != 24000 
        && sample_rate != 32000 
        && sample_rate != 44100 
        && sample_rate != 48000) {
        return -2;
    }

    dp->sample_rate = sample_rate;   
    dp->num_channels = num_channels;

    // Set pointers to delay lines for each channel.
    for (unsigned int i = 0; i < num_channels; ++i) {
        short delay_ms = channel_delays_ms[i];
        if (delay_ms < 0) { // Delay value out of range.
            delay_ms = 0;
        } else if (delay_ms > MAX_DELAY) {
            delay_ms = MAX_DELAY;
        }
        int delay_samples = (int)((delay_ms / 1000.0f) * sample_rate);
        dp->delay[i] = delay_samples;
        dp->delay_lines[i] = (short*) calloc(delay_samples + FRAME_LENGTH, sizeof(short));
        memset(dp->delay_lines[i], 0, (delay_samples + FRAME_LENGTH) * sizeof(short));
    }

    return 0; // Successful initialization.
}

// Process audio data.
int process_audio(DelayProcessorHandle handle,               
                  short (*input_buffer)[FRAME_LENGTH],
                  short (*output_buffer)[FRAME_LENGTH],
                  unsigned int num_frames) {
    struct DelayProcessor* dp = (struct DelayProcessor*) handle;
    if (dp == NULL || input_buffer == NULL || output_buffer == NULL ||
        num_frames != FRAME_LENGTH) {
        return -1; // Invalid parameters or excessive number of frames.
    }

    for (unsigned int ch = 0; ch < dp->num_channels; ++ch) {   
        memcpy(dp->delay_lines[ch] + dp->delay[ch], input_buffer[ch], num_frames * sizeof(short));
        memcpy(output_buffer[ch], dp->delay_lines[ch], num_frames*sizeof(short));       
        memmove(dp->delay_lines[ch], dp->delay_lines[ch]+num_frames, dp->delay[ch]*sizeof(short));
    }
    
    return 0; // Successful processing of audio data.
}

// Destroy the algorithm handle and release resources.
void destroy_delay_processor(DelayProcessorHandle handle) {
    struct DelayProcessor* dp = (struct DelayProcessor*) handle;
    if (dp != NULL) {
        for (int i = 0; i < dp->num_channels; i++) {
            if (dp->delay_lines[i]) {
                free(dp->delay_lines[i]);
                dp->delay_lines[i] = NULL;
            }
        }
        free(dp);
        dp = NULL;
    }
}
