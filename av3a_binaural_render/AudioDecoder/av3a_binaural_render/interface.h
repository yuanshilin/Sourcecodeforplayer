#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "avs3_stat_meta.h"

#ifdef __cplusplus
extern "C" {
#endif


void* CreateRenderer(Avs3MetaData *metadata, int sampleRate, int blockSize);

//int PutPlanarAudioBuffer(void* render, const float **buffer, int frameNum, int channelNum);

int PutInterleavedAudioBuffer(void* render, const float *buffer, int frameNum, int channelNum);

//int GetBinauralPlanarAudioBuffer(void* render, float **buffer, int frameNum);

int GetBinauralInterleavedAudioBuffer(void* render, float *buffer, int frameNum);

int UpdateMetadata(void* render, Avs3MetaData *metadata);

int SetListenerPosition(void* render, float *position, float *front, float *up);

int DestroyRenderer(void* render);


#ifdef __cplusplus
}
#endif


#endif
