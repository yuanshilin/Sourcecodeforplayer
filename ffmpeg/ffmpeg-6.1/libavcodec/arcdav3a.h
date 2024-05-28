
#ifndef DAVS3_DAV3A_DEC_API_H
#define DAVS3_DAV3A_DEC_API_H

#include <stdint.h>
#include "avs3_stat_meta.h"
#include "avs3_stat_dec.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define DAV3A_API __declspec(dllexport)
#else
#define DAV3A_API
#endif

/**
 * ===========================================================================
 * interface struct type defines
 * ===========================================================================
 */

typedef void* (*PFCreateRenderer)(Avs3MetaData *metadata, int sampleRate, int blockSize);
typedef int (*PFPutInterleavedAudioBuffer)(void* render, const float *buffer, int frameNum, int channelNum);
typedef int (*PFGetBinauralInterleavedAudioBuffer)(void* render, float *buffer, int frameNum);
typedef int (*PFUpdateMetadata)(void* render, Avs3MetaData *metadata);
typedef int (*PFSetListenerPosition)(void* render, float *position, float *front, float *up);
typedef int (*PFDestroyRenderer)(void* render);

typedef AVS3DecoderHandle (*PFavs3_create_decoder)();
typedef void (*PFavs3_destroy_decoder)(AVS3DecoderHandle hAvs3Dec);
typedef int (*PFparse_header)(AVS3DecoderHandle hAvs3Dec, unsigned char* pData, int nLenIn, int isInitFrame, int *pnLenConsumed, unsigned short *crc);
typedef int (*PFavs3_decode)(AVS3DecoderHandle hAvs3Dec, unsigned char* pDataIN, int nLenIn, unsigned char* pDataOut, int *pnLenOut, int *pnLenConsumed);

#ifdef __cplusplus
}
#endif

#endif  /// DAVS3_DAVS3_DEC_API_H
