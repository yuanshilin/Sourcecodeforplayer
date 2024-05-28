#define _POSIX_C_SOURCE 199309L  //CLOCK_MONOTONIC ,c99 
#include <stdio.h>
#include <stdlib.h>
#include "avs3_cnst_com.h"
#include "avs3_stat_enc.h"
#include "avs3_prot_enc.h"
#include <time.h>

// #define PROF_ON
#ifdef PROF_ON
#include <gperftools/profiler.h>
#endif

#if defined(WIN32) || defined(_WINDLL)
#include "windows.h"
#endif
long long  VMF_GetTime()
{
    struct timespec time;
    long long  lltime;
#if defined(WIN32) || defined(_WINDLL)
    LARGE_INTEGER count;
    LARGE_INTEGER   freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    lltime = (long long )(count.QuadPart
                         / ((double)freq.QuadPart
                         / 1000000));
#else
    clock_gettime(CLOCK_MONOTONIC, &time);
    lltime = ((long long )time.tv_sec) * 1000000 + time.tv_nsec / 1000;
#endif 
    return lltime;
}

int main(int argc, char* argv[])
{
    static long frame = 1;

    int32_t n;

    FILE* fileInput = NULL;
    FILE* fileBitstream = NULL;
    FILE *fModel = NULL;
    FILE* fileMetadata = NULL;
    AVS3EncoderHandle stAvs3 = NULL;
    int8_t buf[MAX_CHANNELS*MAX_FRAME_LEN * 3];         // 3 for 24bit, 3bytes
    short data[MAX_CHANNELS*MAX_FRAME_LEN];
    short samplesLookahead;
    short numChansInput;

#ifdef  PROF_ON
    ProfilerStart("avs3Encoder_AVX2.prof");
#endif

    if ((stAvs3 = (AVS3EncoderHandle)malloc(sizeof(AVS3Encoder))) == NULL)
    {
        fprintf(stderr, "Can not allocate memory for AVS3 encoder structure!\n");
        exit(-1);
    }

    // command line analysis
    Avs3EncoderGetCommandLine(stAvs3, argc, argv, &fileInput, &fileBitstream, &fileMetadata);

    // encoder init
    Avs3EncoderInit(stAvs3, &fModel);

    samplesLookahead = stAvs3->lookaheadSamples;
    numChansInput = stAvs3->numChansInput;
    long long t0 = VMF_GetTime();
    long long globalSampleCnt = 0;
    long long t1 = VMF_GetTime();
    double t_cost = (t1-t0)/1000.0; //ms 
    long long t_s0 = VMF_GetTime();
    long long t_s1 = VMF_GetTime();
    long long t_s2 = VMF_GetTime();
    long long t_s3 = VMF_GetTime();
   
    long long cost_all_enc  = 0;
    long long cost_all_enc_meta = 0;

    fprintf(stdout, "frame len = %d,samples lookahead = %d\n", stAvs3->frameLength,samplesLookahead);
    while ((n = (int32_t)fread(buf, sizeof(int8_t), (stAvs3->frameLength + samplesLookahead)*numChansInput*(stAvs3->bitDepth >> 3), fileInput)) > 0)
    {
        t_s0 = VMF_GetTime();

        ConvertBitDepth(buf, data, stAvs3->bitDepth, n);

        t_s1 = VMF_GetTime();
        /* metadata encoding */
        Avs3MetadataEnc(stAvs3, fileMetadata);
        t_s2 = VMF_GetTime();

        // frame level encoding
        Avs3Encode(stAvs3, data, n / (stAvs3->bitDepth / 8));       // convert to nSamples
        t_s3 = VMF_GetTime();
        cost_all_enc += t_s3-t_s2;
        cost_all_enc_meta += t_s2 - t_s1;

        /* Write indices to file */
        Avs3FlushBitstream(stAvs3, fileBitstream);
        t1 = VMF_GetTime();
        t_cost = (t1-t0)/1000.0; //ms

        globalSampleCnt += (stAvs3->frameLength+samplesLookahead);
        fprintf(stdout, "%-8ld,%0.3f\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b", frame,globalSampleCnt*1000.0/(48000*t_cost));
        fflush(stdout);

        samplesLookahead = 0;

        frame++;
    }
#ifdef  PROF_ON
  	ProfilerStop();
#endif
    t1 = VMF_GetTime();
    t_cost = (t1-t0)/1000.0; //ms 
    fprintf(stdout, "\n\n");
    fprintf(stdout, "AVS3 Encoder finished...,%0.3f frame/s,%0.3f samples/s,speed=%0.3f,cost all=%0.3f,enc=%0.3f,meta=%0.3f ms\n\n",
                    frame*1000.0/t_cost,globalSampleCnt*1000.0/t_cost,globalSampleCnt*1000.0/(48000*t_cost), 
                    t_cost,cost_all_enc/1000.000,cost_all_enc_meta/1000.000);

    if (fileInput != NULL) 
    {
        fclose(fileInput);
    }

    if (fileBitstream != NULL) 
    {
        fclose(fileBitstream);
    }

    if (fileMetadata != NULL)
    {
        fclose(fileMetadata);
    }

    if (fModel != NULL) 
    {
        fclose(fModel);
    }

    // destroy encoder handle
    Avs3EncoderDestroy(stAvs3);

    return;
}