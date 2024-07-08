#include "delay_processor.h"
#include <stdlib.h>
#include <string.h>

#define MAX_CHANNEL 24
#define MAX_DELAY 200.0f

// Internal data structure encapsulating information related to the delay processor.
struct DelayProcessor {
    MUInt32 sample_rate;
    MUInt32 num_channels;    
    MInt32 delay[MAX_CHANNEL];
    MInt16* delay_lines[MAX_CHANNEL]; 
};

// Create an algorithm handle and allocate frequency division memory.
DelayProcessorHandle create_delay_processor(MVoid) {
    struct DelayProcessor* dp = (struct DelayProcessor*) malloc(sizeof(struct DelayProcessor));
    if (dp == NULL) {
        return NULL;
    }

    dp->sample_rate = 0;
    dp->num_channels = 0;      
    for(MInt32 i = 0; i < MAX_CHANNEL; i++) {
        dp->delay[i] = 0;
        dp->delay_lines[i] = NULL;        
    }

    return (DelayProcessorHandle) dp;
}

// Initialize the algorithm by setting the sample rate, number of channels, and individual channel delays in milliseconds.
MInt32 initialize_delay_processor(DelayProcessorHandle handle,
                                  MUInt32 sample_rate,
                                  MUInt32 num_channels,
                                  const MFloat* channel_delays_ms) {
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
    for (MUInt32 i = 0; i < num_channels; ++i) {
        MInt16 delay_ms = channel_delays_ms[i];
        if (delay_ms < 0) { // Delay value out of range.
            delay_ms = 0;
        } else if (delay_ms > MAX_DELAY) {
            delay_ms = MAX_DELAY;
        }
        MInt32 delay_samples = (MInt32)((delay_ms / 1000.0f) * sample_rate);
        dp->delay[i] = delay_samples;
        dp->delay_lines[i] = (MInt16*) calloc(delay_samples + FRAME_LENGTH, sizeof(MInt16));
        memset(dp->delay_lines[i], 0, (delay_samples + FRAME_LENGTH) * sizeof(MInt16));
    }

    return 0; // Successful initialization.
}

// Process audio data.
MInt32 process_audio(DelayProcessorHandle handle,
                     MInt16 (*input_buffer)[FRAME_LENGTH],
                     MInt16 (*output_buffer)[FRAME_LENGTH],
                     MUInt32 num_frames) {
    struct DelayProcessor* dp = (struct DelayProcessor*) handle;
    if (dp == NULL || input_buffer == NULL || output_buffer == NULL ||
        num_frames != FRAME_LENGTH) {
        return -1; // Invalid parameters or excessive number of frames.
    }

    for (MUInt32 ch = 0; ch < dp->num_channels; ++ch) {   
        memcpy(dp->delay_lines[ch] + dp->delay[ch], input_buffer[ch], num_frames * sizeof(MInt16));
        memcpy(output_buffer[ch], dp->delay_lines[ch], num_frames*sizeof(MInt16));       
        memmove(dp->delay_lines[ch], dp->delay_lines[ch]+num_frames, dp->delay[ch]*sizeof(MInt16));
    }
    
    return 0; // Successful processing of audio data.
}

// Destroy the algorithm handle and release resources.
void destroy_delay_processor(DelayProcessorHandle handle) {
    struct DelayProcessor* dp = (struct DelayProcessor*) handle;
    if (dp != NULL) {
        for (MInt32 i = 0; i < dp->num_channels; i++) {
            if (dp->delay_lines[i]) {
                free(dp->delay_lines[i]);
                dp->delay_lines[i] = NULL;
            }
        }
        free(dp);
        dp = NULL;
    }
}
