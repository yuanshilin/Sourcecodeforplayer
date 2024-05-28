#ifndef AVS3_PROT_COM_H
#define AVS3_PROT_COM_H

#include <stdio.h>
#include "Instruction.h"
#include "avs3_options.h"
#include "avs3_stat_com.h"
#include "avs3_cnst_com.h"

short GetFrameLength(const long fs);

char *toUpper(char *str);

void GetMdctWindow(float* win, const short len);

void GetWindowShape(AVS3_CORE_CONFIG_DATA_HANDLE hCoreConfig, const short transformType, float* winLeft, float*winRight);

void WindowSignal(AVS3_CORE_CONFIG_DATA_HANDLE hCoreConfig, const float* signal, float* winSignal, const short transformType, const float* winLeft, const float*winRight);

// mdct spectrum grouping
void MdctSpectrumInterleave(
    float *mdctSpectrum,
    int16_t length,
    int16_t numShortBlock
);

// mdct spectrum degrouping
void MdctSpectrumDeinterleave(
    float *mdctSpectrum,
    int16_t length,
    int16_t numShortBlock
);

void SpectrumGroupingEnc(
    float *mdctSpectrum,
    int16_t length,
    int16_t transformType,
    int16_t *groupIndicator,
    int16_t *numGroups
);

void SpectrumDegroupingDec(
    float *mdctSpectrum,
    int16_t length,
    int16_t transformType,
    int16_t *groupIndicator
);

int32_t GetAvailableBits(
    int32_t bitsPerFrame,
    int32_t bitsUsed,
    short *numGroups,
    short nChans,
    NnTypeConfig nnTypeConfig
);

// crc16 check function
uint16_t Crc16(
    uint8_t * bitstream,
    uint32_t bytesFrame
);

void InitCoreConfig(AVS3_CORE_CONFIG_DATA_HANDLE hCoreConfig, const short frameLength);

void MDCT(float *signal, float* output, const short N);

void IMDCT(float *signal, const short N);

void FFT(float *xr, float *xi, const short N);

void IFFT( float *xr, float *xi, const short size);

void SetZero(float *vec, const short len);

void SetFloat(float y[], const float val, const short N);

void SetShort(short y[], const short a, const short N);

void SetUShort(unsigned short y[], const unsigned short a, const short N);

void SetUC(uint8_t y[], const uint8_t a, const short N);

void Mvf2f(const float x[], float y[], const short n);

float VLinalgNorm(float* vec, const short len);

float Dotp(const float  x[], const float  y[], const short  n);

void MvShort2Short(const short x[], short y[], const short n);

unsigned long MvFloat2Short(const float x[], short y[], const short n);

void Vadd(const float x1[], const float x2[], float y[], const short N);

void VMult(const float x1[], const float x2[], float y[], const short N);

void SwapS(short *a, short *b);

void SortS(short *x, const short len);

float SumFloat(const float *x, const short len);

void VMultC(const float x[], const float c, float y[], const short N);

unsigned long Avs3SynthOutput(float synth[MAX_CHANNELS][BLOCK_LEN_LONG], const short output_frame, const short n_channels, short *synth_out);

void Avs3HoaInitConfig(AVS3_HOA_CONFIG_DATA_HANDLE hConfig, const short numChansInput, const short lenFrame, const short coreBwidth, const long totalBitrate);

void HoaBitrateConfigTable(AVS3_HOA_CONFIG_DATA_HANDLE hConfig);

void GetNeighborBasisCoeff(const short anglePair[L_SECOND_ORDER_MP_BASIS][2], float basisCoeffs[L_SECOND_ORDER_MP_BASIS][MAX_HOA_CHANNELS]);

void GetSingleNeighborBasisCoeff(const short anglePair[2], float basisCoeffs[L_HOA_BASIS_ROWS]);

void HoaSplitBytesGroup(AVS3_HOA_CONFIG_DATA_HANDLE hConfig, short* bytesChannels, short* groupRatio, short ratio[MAX_HOA_DMX_GROUPS][MAX_HOA_DMX_CHANNELS], const short totalBits);

// Stereo mode functions
float CalculateEnergyRatio(
    float *x0,
    float *x1,
    const short frameLengh
);

void ComputeBitsRatio(
    float *x0,
    float *x1,
    const short frameLength,
    const short isMs,
    short *bitsRatio
);

void StereoBitsAllocation(
    short availableBits,
    short bitsRatio,
    short *channelBytes
);

void StereoMsProcess(
    float *x0,
    float *x1,
    const short frameLength,
    short *ild
);

void StereoInvMsProcess(
    float *x0,
    float *x1,
    const short frameLength,
    short ild
);


float Mean(
    const float *vec,
    const short lvec
);

void SetC(
    int8_t y[],
    const int8_t a,
    const short N
);

void Index2PairMapping(
    AVS3_MC_PAIR_DATA_HANDLE hPair,
    const short index,
    const short channelNum
);

short Pair2IndexMapping(
    const short ch1,
    const short ch2,
    const short channelNum
);

void GetChRatio(
    short ChannelNum,
    float *mdctSpectrum[MAX_CHANNELS],
    short chBitRatios[MAX_CHANNELS],
    const int totalBrate,
    const int reAllocEna
);

void McBitsAllocation(
    int32_t totalBits,
    const short splitRatio[MAX_CHANNELS],
    short ChannelNum,
    short* channelBytes,
    short lfeExist,
    short lfeBytes
);

int32_t McBitsAllocationHasSiL(
    int32_t totalBits,
    const short splitRatio[MAX_CHANNELS],
    short channelNum,
    short* channelBytes,
    short silFlag[MAX_CHANNELS],
    short lfeExist,
    short lfeBytes
);

void CalcChannelEnergies(
    float *spectrum[MAX_CHANNELS],
    float energy[MAX_CHANNELS],
    const short nchan
);

void Avs3McacDec(
    AVS3_MC_DEC_HANDLE hMcac
);

short GetLfeAllocBytes(
    const int32_t totalBitrate,
    const int16_t coupleChannels
);

void McLfeProc(
    float *mdctSpectrum
);


// cnn layer
int16_t InitCnnLayer(
	modul_structure *fModel,
    CnnStructHandle cnnLayer,
    int16_t isTranspose,
    int16_t featDimIn
);

int16_t DestroyCnnLayer(
    CnnStructHandle cnnLayer
);

int16_t Conv1D(
    CnnStructHandle cnnLayer,
    float *featureIn
);

int16_t Conv1DTranspose(
    CnnStructHandle cnnLayer,
    float *featureIn
);

int16_t Conv1DTranspose2Part(
    CnnStructHandle cnnLayer,
    float *featureIn
);

// Neural codec init
int16_t InitNeuralCodec(
	modul_structure *fModel,
    NeuralCodecHandle *baseCodecSt,
    NeuralCodecHandle *contextCodecSt,
    TypeModel modelType
);

// Neural codec destroy
int16_t DestroyNeuralCodec(
    NeuralCodecHandle *baseCodecSt,
    NeuralCodecHandle *contextCodecSt
);

// Neural QC data init
int16_t InitNeuralQcData(
    NeuralQcData *neuralQcData
);

// top interface for encoder, Hyper
int16_t MdctQuantEncodeHyper(
    NeuralCodecHandle baseCodecSt,
    NeuralCodecHandle contextCodecSt,
    NeuralQcData *neuralQcData,
    float mdctCoefs[][2],
    int16_t numLinesEncode,
    int16_t numChannels,
    int16_t numLinesNonZero,
    int16_t numGroups,
    int16_t *groupIndicator,
    int16_t targetNumBytes
);

int16_t MdctQuantEncodeHyperLc(
    NeuralCodecHandle baseCodecSt,
    NeuralCodecHandle contextCodecSt,
    NeuralQcData *neuralQcData,
    float mdctCoefs[][2],
    int16_t numLinesEncode,
    int16_t numChannels,
    int16_t numLinesNonZero,
    int16_t numGroups,
    int16_t *groupIndicator,
    float *prevFeatureScale,
    int16_t targetNumBytes
);

// top interface for decoder, Hyper
int16_t MdctDequantDecodeHyper(
    NeuralCodecHandle baseCodecSt,
    NeuralCodecHandle contextCodecSt,
    NeuralQcData *neuralQcData,
    float featureDec[][2],
    int16_t numLinesNoiseFill,
    int16_t numGroups,
    int16_t *groupIndicator
);

int16_t MdctDequantDecodeHyperLc(
    NeuralCodecHandle baseCodecSt,
    NeuralCodecHandle contextCodecSt,
    NeuralQcData *neuralQcData,
    float featureDec[][2],
    int16_t numLinesNoiseFill,
    int16_t numGroups,
    int16_t *groupIndicator
);

// Noise filling funcs
void ExtractNfParam(
    float *origLatent,
    int32_t *quantizedLatent,
    float *quantileMedian,
    int16_t numLatentDim,
    int16_t numLatentChannels,
    int16_t numNfDim,
    int16_t numGroups,
    int16_t *groupIndicator,
    float *nfParamQ,
    int16_t *nfParamQIdx
);

void NfParamPostProc(
    NeuralQcData *neuralQcData,
    float *mdctSpectrum,
    int16_t numLinesForNf,
    int32_t totalBitrate,
    int16_t nChans
);

void LatentNoiseFilling(
    float *dequantizedLatent,
    float *quantileMedian,
    int16_t numLatentDim,
    int16_t numLatentChannels,
    int16_t numNfDim,
    int16_t numGroups,
    int16_t *groupIndicator,
    float *nfParamQ,
    int16_t *nfParamQIdx
);

// quantization funcs
int16_t InitQuantizer(
	modul_structure *fModel,
    QuantizerHandle quantileMedianParam,
    int16_t numChannels
);

int16_t LatentQuantize(
    QuantizerHandle quantizerHandle,
    float *featureIn,
    int32_t *featureOut,
    int16_t featureDim,
    int16_t numChannels
);

int16_t LatentDequantize(
    QuantizerHandle quantizerHandle,
    int32_t *featureIn,
    float *featureOut,
    int16_t featureDim,
    int16_t numChannels
);

int16_t DestroyQuantizer(
    QuantizerHandle quantizerHandle
);

// Range Coder funcs
int16_t InitRangeCoderConfig(
	modul_structure *fModel,
    RangeCoderConfigHandle rangeCoderConfig,
    int16_t numCdfs
);

int16_t DestroyRangeCoderConfig(
    RangeCoderConfigHandle rangeCoderConfig
);

int16_t InitRangeEncoderState(
    RangeEncoderStateHandle rangeEncoderSt
);

void RangeEncodeProcess(
    RangeCoderConfigHandle rangeCoderConfig,
    int32_t *data,
    int16_t dataLength,
    int16_t *index,
    unsigned char *sink,
    int16_t *sinkPointer
);

void RangeEncodeProcessBrEst(
    RangeCoderConfigHandle rangeCoderConfig,
    int32_t *data,
    int16_t dataLength,
    int16_t *index,
    int16_t *sinkPointer
);

int16_t InitRangeDecoderState(
    RangeDecoderStateHandle rangeDecoderSt
);

void RangeDecodeProcess(
    RangeCoderConfigHandle rangeCoderConfig,
    int32_t *data,
    int16_t dataLength,
    int16_t *index,
    unsigned char *input,
    int16_t inputLength
);

// activation funcs
void ApplyReluActFuncVec(
    const float *srcVec,
    int16_t len,
    float *destVec
);

void ApplyReluActFunc2D(
    const float **srcMat,
    int16_t numRow,
    int16_t numCol,
    float **destMat
);

void ApplyLinearActFuncVec(
    const float *srcVec,
    int16_t len,
    float *destVec
);

void ApplyLinearActFunc2D(
    const float **srcMat,
    int16_t numRow,
    int16_t numCol,
    float **destMat
);

void ApplySigmoidActFuncVec(
    const float *srcVec,
    int16_t len,
    float *destVec
);

void ApplySigmoidActFunc2D(
    const float **srcMat,
    int16_t numRow,
    int16_t numCol,
    float **destMat
);

void ApplyTanhActFuncVec(
    const float *srcVec,
    int16_t len,
    float *destVec
);

void ApplyTanhActFunc2D(
    const float **srcMat,
    int16_t numRow,
    int16_t numCol,
    float **destMat
);

int16_t InitGdnParam(
	modul_structure *fModel,
    GdnActFuncHandle gdnActFuncParam,
    int16_t numChannelsOut
);

int16_t DestroyGdnParam(
    GdnActFuncHandle gdnActFuncParam,
    int16_t numChannelsOut
);

void ApplyGdnActFunc(
    GdnActFuncHandle gdnActFuncParam,
    const float *featureIn,
    int16_t dimFeat,
    int16_t numChannel,
    float *featureOut
);

void ApplyIgdnActFunc(
    GdnActFuncHandle gdnActFuncParam,
    const float *featureIn,
    int16_t dimFeat,
    int16_t numChannel,
    float *featureOut
);

// FD shaping and LPC ralated funcs
void LsfToLsp(
    const float *lsf,
    float *lsp,
    const short lpcOrder,
    const int samplingRate
);

void LspToLpc(
    const float *lsp,
    float *lpc,
    const short lpcOrder
);

void SpectrumShaping(
    float *mdctSpectrum,
    float *lpcQuantCoeffs,
    float *lpcGain,
    const short lpcOrder,
    const short len,
    const short noInverse
);

void LsfQuantDecHbr(
    const short *vqIndex,
    float *quantLsf,
    const short lpcOrder
);

void LsfQuantDecLbr(
    const short *vqIndex,
    float *quantLsf,
    const short lpcOrder
);

void LsfQuantDec(
    const short *vqIndex,
    float *quantLsf,
    const short lpcOrder,
    const short lsfLbrFlag
);

void Avs3FdInvSpectrumShaping(
    short *lsfVqIndex,
    float *mdctSpectrum,
    short lsfLbrFlag
);

// TNS functions
void TnsParaInit(
    TnsData * pTnsData
);

void TnsEnc(
    TnsData * pTnsData,
    float *origSpectrum,
    const short isShortFrame
);

void TnsDec(
    TnsData * pTnsData,
    float *origSpectrum,
    const short isShortFrame
);

// bwe functions
int16_t GetBwePresent(
    const int16_t avs3Format,
    const int32_t totalBitrate,
    const int16_t numChannels
);

void BweGetConfig(
    BweConfigHandle bweConfig,
    const int16_t avs3Format,
    const int32_t totalBitrate,
    const int16_t numChannels
);

// bwe tool functions
float BweGetSfm(
    float *enerSpec,
    float *logEnerSpec,
    int16_t start,
    int16_t stop
);

float BweGetPeakAvgRatio(
    float *logEnerSpec,
    int16_t start,
    int16_t stop
);

// bwe dec functions
void InitBweDecData(
    BweDecDataHandle bweDecData
);

void BweApplyDec(
    BweConfigHandle bweConfig,
    BweDecDataHandle bweDecData,
    float *mdctSpectrum
);

// MCR functions
void InitMcrConfig(
    MCR_CONFIG_HANDLE mcrConfig
);

void InitMcrData(
    MCR_DATA_HANDLE mcrData
);

int16_t McrEncode(
    MCR_DATA_HANDLE mcrData,
    MCR_CONFIG_HANDLE mcrConfig,
    float *mdctSpectrumL,
    float *mdctSpectrumR,
    int16_t isShortWin
);

int16_t McrDecode(
    MCR_DATA_HANDLE mcrData,
    MCR_CONFIG_HANDLE mcrConfig,
    float *mdctSpectrumL,
    float *mdctSpectrumR,
    int16_t isShortWin
);

// matrix mult using GEMM
void MatrixMultGemm(
    const MatrixStruct *matA,
    const MatrixStruct *matB,
    MatrixHandleUnconst matM);

// #ifdef AVS_NEON_ON
#if defined(_NEON) && defined(SUPPORT_NEON)
void MatrixMultGemmNeon(
    const MatrixStruct *matA,
    const MatrixStruct *matB,
    MatrixHandleUnconst matM);

#endif

#if defined(_AVX2) && defined(SUPPORT_AVX2)
void MatrixMultGemmAvx2(
    const MatrixStruct *matA,
    const MatrixStruct *matB,
    MatrixHandleUnconst matM);
#endif

#if defined(_AVX512) && defined(SUPPORT_AVX512)
void MatrixMultGemmAvx512(
    const MatrixStruct *matA,
    const MatrixStruct *matB,
    MatrixHandleUnconst matM);
#endif

#endif
