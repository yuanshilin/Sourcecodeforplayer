#include "delay_processor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const unsigned int num_frames = 1024; 
#define CH 2
#define SAMPLE_RATE 44100
int main(int argc, const char * argv[]) {
    FILE *fin = fopen("/Users/develop/Downloads/audio_delay/test_out_before.pcm", "rb");
    FILE *fout = fopen("/Users/develop/Downloads/audio_delay/test_out_before_after.pcm", "wb");
    if (fin == NULL || fout == NULL) {
        printf("open file failed\n");
        exit(1);
    }

    printf("%d, %d\r\n", sizeof(short), sizeof(short*));
    short inbuf[CH*1024];
    short outbuf[CH *1024];
    short input_data[CH][1024];
    short output_data[CH][1024];

    DelayProcessorHandle handle = create_delay_processor();
    if (handle == NULL) {
        printf("Failed to create delay processor.\n");
        return -1;
    }

    // generate test data
    float channel_delays_ms[CH];
    for (int i = 0; i < CH; i++) {
        channel_delays_ms[i] = (i) * 100.0f;// +100.0f;
    }

    if (initialize_delay_processor(handle, SAMPLE_RATE, CH, channel_delays_ms) != 0) {
        printf("Failed to initialize delay processor.\n");
        destroy_delay_processor(handle);
        return -1;
    }

    while(!feof(fin)) {

        if(fread(inbuf, sizeof(short), num_frames* CH, fin)!= num_frames*CH) {
            break;
        }

        for (int ch = 0;ch < CH; ch++) {
            for (int i = 0; i < num_frames; i++) {
                input_data[ch][i] = inbuf[CH *i + ch];
            }
        }

        if (process_audio(handle, input_data, output_data, num_frames) != 0) {
            printf("Failed to process audio.\n");
            destroy_delay_processor(handle);
            return -1;
        }
        for (int ch = 0;ch < CH; ch++) {
            for (int i = 0; i < num_frames; i++) {
                outbuf[CH *i + ch] = output_data[ch][i];
            }
        }
        fwrite(outbuf, sizeof(short), num_frames* CH, fout);
    }
    destroy_delay_processor(handle);
    return 0;
}
