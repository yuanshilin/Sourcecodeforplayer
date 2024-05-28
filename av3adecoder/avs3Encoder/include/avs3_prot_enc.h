#ifndef AVS3_PROT_ENC_H
#define AVS3_PROT_ENC_H

#include <stdio.h>

#include "avs3_options.h"
#include "avs3_stat_enc.h"

void Avs3EncoderGetCommandLine(
    AVS3EncoderHandle stAvs3, 
    int argc, 
    char *argv[], 
    FILE  **fileInput, 
    FILE  **fileBitstream,
    FILE  **fileMetadata
);

void ConvertBitDepth(int8_t* buf, short* data, const short bitDepth, const int32_t samples);

void Avs3EncoderInit(AVS3EncoderHandle stAvs3,FILE** fModel);

void Avs3EncCreateStereo(AVS3EncoderHandle stAvs3);

void Avs3EncCreateMono(AVS3EncoderHandle stAvs3);

void Avs3Encode(AVS3EncoderHandle stAvs3, const short *data, const short samples);

void Avs3CoreEncode(AVS3EncoderHandle stAvs3, float data[MAX_CHANNELS][MAX_FRAME_LEN], const short lenFrame, const short nChans);

void Avs3EncoderDestroy(AVS3EncoderHandle stAvs3);

void Avs3PreAnalysis(AVS3EncoderHandle stAvs3, const short nChans, const short LenFrame);

void McMixGetSilenceFlag(
    AVS3EncoderHandle stAvs3,
    const short nChans,
    const short lenFrame);

// trans detection functions
void InitWindowTypeDetect(const short frameLength, WindowTypeDetectData *winTypeDetector);

int16_t WindowTypeDetect(WindowTypeDetectData *winTypeDetector, float const * inPut, const short frameLength, const short initFrame);

void CoreSignalAnalysis(AVS3EncoderHandle stAvs3, const short nChans, const short lenFrame);

void Avs3LocalDecoder(AVS3_ENC_CORE_HANDLE hEncCore, float output[BLOCK_LEN_LONG]);

// HOA functions
void Avs3HOAEncoder(AVS3EncoderHandle stAvs3, float data[MAX_CHANNELS][MAX_FRAME_LEN], const short lenFrame);

void Avs3HOAReconfig(AVS3EncoderHandle stAvs3,short* nChans);

void Avs3EncCreateHoa(AVS3EncoderHandle stAvs3);

int Avs3HoaSVD(float a[HOA_LEN_FRAME48k][L_HOA_BASIS_ROWS], int m, int n, float* w, float v[HOA_LEN_FRAME48k][L_HOA_BASIS_ROWS]);

// stereo functions
void Avs3StereoEncoder(
    AVS3EncoderHandle stAvs3,
    short* channelBytes
);

void Avs3StereoMcrEncoder(
    AVS3EncoderHandle stAvs3,
    short* channelBytes
);

// mono functions
void Avs3MonoEncoder(
    AVS3EncoderHandle stAvs3,
    short* channelBytes
);

void Avs3EncCreateMc(
    AVS3EncoderHandle stAvs3
);

void Avs3McEncoder(
    AVS3EncoderHandle stAvs3,
    short* channelBytes
);

void Avs3EncCreateMix(
    AVS3EncoderHandle stAvs3
);

void Avs3MixEncoder(
    AVS3EncoderHandle stAvs3,
    short* channelBytes
);

void Avs3MetadataEnc(
    AVS3EncoderHandle stAvs3, 
    FILE* fMetadata
);

void Avs3HoaCoreEncoder(AVS3EncoderHandle stAvs3, short* channelBytes);

/* LPC analysis and spectrum shaping */
void Avs3FdSpectrumShaping(
    AVS3_ENC_CORE_HANDLE hEncCore,
    int16_t chIdx
);

void LsfQuantEnc(
    const float *lsf,
    float *quantLsf,
    short *lsfVqIndex,
    const short lpcOrder,
    const short lsfLbrFlag
);

// bwe functions
void InitBweEncData(
    BweEncDataHandle bweEncData
);

void BweApplyEnc(
    BweConfigHandle bweConfig,
    BweEncDataHandle bweEncData,
    float *mdctSpectrum,
    float *powerSpectrum,
    int16_t isLongWin
);

typedef struct bitstreambuf
{
	char* buf;
	int len;
}bitstreambuf;

void Avs3FlushBitstream2buf(AVS3EncoderHandle stAvs3, bitstreambuf *pBuf);

void Avs3FlushBitstream(AVS3EncoderHandle stAvs3, FILE *fBitstream);

void ResetIndicesEnc(AVS3_BSTREAM_ENC_HANDLE bsHandle, int16_t maxNumIndices);

void PushNextIndice(AVS3_BSTREAM_ENC_HANDLE bsHandle, uint16_t value, int16_t totalSideBits);

void IndicesToSerial(const Indice *indiceList, const int16_t numIndices, uint8_t *bitstream, uint32_t *bitstreamSize);

void WriteCoreSideBitstream(AVS3EncoderHandle stAvs3, int16_t nChans, uint8_t *bitstream, uint32_t *bitstreamSize);

void WriteGroupBitstream(
    AVS3EncoderHandle stAvs3,
    int16_t nChans,
    uint8_t *bitstream,
    uint32_t *bitstreamSize
);

void WriteQcBitstream(AVS3EncoderHandle stAvs3, int16_t nChans, uint8_t *bitstream, uint32_t *bitstreamSize);

void WriteStereoBitstream(AVS3EncoderHandle stAvs3, uint8_t *bitstream, uint32_t *bitstreamSize);

void WriteMcBitstream(
    AVS3EncoderHandle stAvs3, 
    uint8_t *bitstream, 
    uint32_t *bitstreamSize, 
    short chBitRatios[MAX_CHANNELS]
);

void WriteHoaBitstream(AVS3EncoderHandle stAvs3);

#endif // AVS3_PROT_ENC_H
