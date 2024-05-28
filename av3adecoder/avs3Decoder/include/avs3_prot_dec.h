#ifndef AVS3_PROT_DEC_H
#define AVS3_PROT_DEC_H

#include <stdio.h>
#include "avs3_stat_dec.h"
#include "avs3_cnst_com.h"

short Avs3ParseBsFrameHeader(
    AVS3DecoderHandle hAvs3Dec,
    uint8_t *headerBs,
    int16_t isInitFrame,
    uint16_t *crcBs
);

FILE* WriteWavHeader(const char* fileName, const short nChans, const long fs);

void WriteSynthData(const short* data, FILE* file, const short nChans, const short frameLength);

void SynthWavHeader(FILE* file);

int Avs3InitDecoder(AVS3DecoderHandle hAvs3Dec, FILE** fModel, const char *modelPath);

void Avs3DecoderDestroy(AVS3DecoderHandle hAvs3Dec);

void ResetBitstream(AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream);

// short ReadBitstream(AVS3DecoderHandle hAvs3Dec, FILE* fBitstream);

uint16_t GetNextIndice(uint8_t *bitstream, uint32_t *nextBitPos, int16_t numBits);

void DecodeStereoSideBits(AVS3DecoderHandle hAvs3Dec, AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream);

void DecodeMcSideBits(
    AVS3_MC_DEC_HANDLE hDecMc,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream,
    short chBitRatios[MAX_CHANNELS]
);

void DecodeHoaSideBits(AVS3_HOA_DEC_DATA_HANDLE hDecHoa, AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream);

void DecodeCoreSideBits(AVS3_DEC_CORE_HANDLE hDecCore, AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream);

void DecodeGroupBits(AVS3_DEC_CORE_HANDLE hDecCore, AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream);

void DecodeQcBits(
    AVS3_DEC_CORE_HANDLE hDecCore,
    NnTypeConfig nnTypeConfig,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream,
    const short channelBytes);

void Avs3McDec(AVS3DecoderHandle hAvs3Dec, float synth[MAX_CHANNELS][FRAME_LEN]);

void Avs3StereoDec(AVS3DecoderHandle hAvs3Dec, float synth[MAX_CHANNELS][FRAME_LEN]);

void Avs3StereoMcrDec(AVS3DecoderHandle hAvs3Dec, float synth[MAX_CHANNELS][FRAME_LEN]);

void Avs3MonoDec(AVS3DecoderHandle hAvs3Dec, float synth[MAX_CHANNELS][FRAME_LEN]);

void Avs3MixDec(AVS3DecoderHandle hAvs3Dec, float synth[MAX_CHANNELS][FRAME_LEN]);

void Avs3HoaDec(AVS3DecoderHandle hAvs3Dec, float synth[MAX_CHANNELS][FRAME_LEN]);

void Avs3InverseQC(AVS3DecoderHandle hAvs3Dec, short nChans);

void Avs3PostSynthesis(
    AVS3_DEC_CORE_HANDLE hDecCore,
    float *synth,
    short isLfe
);

void Avs3MetadataDec(AVS3DecoderHandle hAvs3Dec);

void Avs3Decode(AVS3DecoderHandle hAvs3Dec, short data[MAX_CHANNELS * FRAME_LEN]);

void Avs3InverseMdctDecoder(AVS3_DEC_CORE_HANDLE hEncCore, float output[BLOCK_LEN_LONG]);

#endif