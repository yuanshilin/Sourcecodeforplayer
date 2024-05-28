#ifndef AVS3_STAT_COM_H
#define AVS3_STAT_COM_H

#include <stdint.h>
#include <string.h>

#include "avs3_options.h"
#include "avs3_cnst_com.h"

typedef struct avs3_wav_header_structure {

    int8_t chunkID[4]; 
    int32_t riffSize; 
    int8_t format[4]; 
    int8_t subchunkID[4]; 
    int32_t subchunkSize; 
    uint16_t audioFormat;
    uint16_t numChannels;
    int32_t sampleRate;
    int32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitDepth;
    int8_t dataID[4];
    int32_t dataSize;
} AVS3_WAVE_HEADER_DATA, *AVS3_WAVE_HEADER_DATA_HANDLE;

typedef struct avs3_wav_chunk_structure {

    int8_t chunkID[4];
    int32_t chunkSize;

} AVS3_WAVE_CHUNK_DATA, *AVS3_WAVE_CHUNK_DATA_HANDLE;

// channel config table
// for mono/stereo/mc/hoa
typedef enum _ChannelNumConfig {
    CHANNEL_CONFIG_MONO = 0,
    CHANNEL_CONFIG_STEREO = 1,
    CHANNEL_CONFIG_MC_5_1,
    CHANNEL_CONFIG_MC_7_1,
    CHANNEL_CONFIG_MC_10_2,
    CHANNEL_CONFIG_MC_22_2,
    CHANNEL_CONFIG_MC_4_0,
    CHANNEL_CONFIG_MC_5_1_2,
    CHANNEL_CONFIG_MC_5_1_4,
    CHANNEL_CONFIG_MC_7_1_2,
    CHANNEL_CONFIG_MC_7_1_4,
    CHANNEL_CONFIG_HOA_ORDER1,
    CHANNEL_CONFIG_HOA_ORDER2,
    CHANNEL_CONFIG_HOA_ORDER3,
    CHANNEL_CONFIG_UNKNOWN
}ChannelNumConfig;

/* Codec bitrate config struct */
typedef struct CodecBitrateConfigStructure
{
    ChannelNumConfig channelNumConfig;
    const long *bitrateTable;
}CodecBitrateConfig;

/* MC channel config table */
/* Cmd line, ChannelNumConfig and numChannels mapping */
typedef struct McChannelConfigStructure
{
    const char mcCmdString[10];
    ChannelNumConfig channelNumConfig;
    const int16_t numChannels;
}McChanelConfig;

// Neural network model type configuration
typedef enum _NnTypeConfig {
    NN_TYPE_DEFAULT_MAIN = 0,
    NN_TYPE_DEFAULT_LC
}NnTypeConfig;

typedef struct avs3_core_data_structrue
{
    float longWindow[BLOCK_LEN_LONG];
    float shortWindow[BLOCK_LEN_SHORT];

    short overlapShortSize;
    short overlapLongSize;
    short overlapPaddingSize;

} AVS3_CORE_CONFIG_DATA, *AVS3_CORE_CONFIG_DATA_HANDLE;

typedef struct avs3_hoa_group_configure
{
    long bitrate;
    short spatialAnalysis;
    short nGroups;
    short dmxGroup[MAX_HOA_DMX_GROUPS];
    short coreBwidth[MAX_HOA_DMX_GROUPS];
    float bitsRatio[MAX_HOA_DMX_GROUPS];
    float maxBitsRatio;
    short groupBwe[MAX_HOA_DMX_GROUPS];

} HOA_GROUP_CONFIG;

typedef struct avs3_hoa_config_data_struct
{
    /* core information. */
    long totalBitrate;
    short frameLength;

    /* HOA configure */
    short order;
    short nTotalChansInput;
    short nTotalChansTransport;
    short nTotalForeChans;
    short nTotalResChans;
    short maxCodeLines;
    short spatialAnalysis;

    /* channel group */
    short nTotalChanGroups;
    short groupChans[MAX_HOA_DMX_GROUPS];
    short groupCodeLines[MAX_HOA_DMX_GROUPS];
    float groupBitsRatio[MAX_HOA_DMX_GROUPS];
    short groupIndexBits[MAX_HOA_DMX_GROUPS];

    float maxDirecChanBitsRatio;

    /* channel offset for FOA/HOA */
    short groupChOffset[MAX_HOA_DMX_GROUPS + 1];

    /* BWE configure for group */
    short groupBwe[MAX_HOA_DMX_GROUPS];

    short overlapSize;
    float hoaWindow[HOA_OVERLAP_SIZE];

    short innerFormat;

}AVS3_HOA_CONFIG_DATA,*AVS3_HOA_CONFIG_DATA_HANDLE;


/* MCAC PAIR ILD MODE */
typedef enum {
    PAIR_ILD_MODE = 0,          /* the paired two channels with the same ILD energy, different pairs have different ILD energies */
    ALL_ILD_MODE = 1            /* all paired channels with the same ILD energy */
} McIldMode;

/* MCAC coupling sort struct */
typedef struct {
    short ch1;          /* first channel  */
    short ch2;          /* second channel */
    float xcorr;        /* corrletion coeffient between ch1 and ch2 */
} McacSort;

typedef struct avs3_mc_pair_data_structure
{
    short ch1;
    short ch2;
} AVS3_MC_PAIR_DATA, *AVS3_MC_PAIR_DATA_HANDLE;

/* AVS3 MC DEC structure */
typedef struct avs3_mc_dec_data_structure
{
    float* mcSpectrum[MAX_CHANNELS];
    short mcSideBits;

    short channelNum;                                       // channel num
    short coupleChNum;                                      // channel num without LFE
    short isMixed;                                          // 1 for bed+obj or pure obj, 0 for only sound bed
    short objNum;                                           // object number
    short hasSilFlag;                                       // set to 0 if no silence frame in all channels
    short silFlag[MAX_CHANNELS];                            // silFlag for each channel
    short lfeChIdx;                                         // LFE channel idx
    short lfeExist;                                         // if have LFE channel
    short lfeBytes;                                         // LFE allocated bytes per frame
    unsigned short mcIld[MAX_CHANNELS];                     // ILD parameter, bs used
    short pairCnt;                                          // pairing num, bs used
    short bitsPairIndex;                                    // bits needed to code channel pair index, depends on number of active channels
    AVS3_MC_PAIR_DATA hPair[MAX_CHANNELS / 2];              // pairing structure , bs use index

}AVS3_MC_DEC_DATA, *AVS3_MC_DEC_HANDLE;


// Quantizer structure
typedef struct QuantizerStructure {
    int16_t numChannels;
    float *quantileMedian;
}QuantizerStruct, *QuantizerHandle;

// Range coder config
typedef struct RangeCoderConfigStructure {
    uint16_t numCdfs;                           // number of CDFs
    uint16_t *cdfLength;                        // CDF length vector, size: numCdfs
    int16_t  *offset;                           // offset for each cdf to map input data to [0, cdfLength-2]
    uint32_t **quantizedCdf;                    // quantized CDF table, size: numCdfs * maxCdfLength
    uint16_t precision;                         // precision of CDF table in bits, range (0, 16]
    uint16_t overflowWidth;                     // number of bits for each section of overflow coding
}RangeCoderConfigStruct, *RangeCoderConfigHandle;

// Range encoder state
typedef struct RangeEncoderStateStructure {
    uint32_t base;
    uint32_t sizeMinusOne;
    uint64_t delay;
}RangeEncoderStateStruct, *RangeEncoderStateHandle;

// Range decoder state
typedef struct RangeDecoderStateStructure {
    uint32_t base;
    uint32_t sizeMinusOne;
    uint32_t value;
}RangeDecoderStateStruct, *RangeDecoderStateHandle;

// model type definition
typedef enum _TypeModel {
    VAE,
    HYPER
}TypeModel;

// type of activation functions
typedef enum _TypeActFunc {
    RELU = 0,
    LINEAR,
    SIGMOID,
    TANH,
    GDN,
    IGDN,
    DN,
    IDN
}TypeActFunc;

// type of padding before cnn
typedef enum _TypePadding {
    SAME = 0,
    VALID
}TypePadding;

// gdn params structure
typedef struct GdnActFuncStructure {

    float *beta;
    float *gamma;

}GdnActFuncStruct, *GdnActFuncHandle;

// cnn layer structure for 1D
typedef struct CnnLayerStructure {

    int16_t isTranspose;                // flag for transpose convolution, 1 for transpose

    int16_t numChannelsIn;              // number of input channels
    int16_t numChannelsOut;             // number of output channels
    int16_t kernelSize;                 // kernel size
    int16_t useBias;                    // flag for bias usage, 1 for use
    int16_t stride;                     // stride for conv, downsample for conv, upsample for conv_transpose
    TypePadding padding;                // padding type

    float ***kernel;                    // kernel, size for encoder: kernelSize * numChannelsIn * numChannelsOut
                                        // size for decoder: kernelSize * numChannelsOut * numChannelsIn
    float *bias;                        // bias, size: numChannelsOut

    TypeActFunc activationFunc;         // type of activation function

    GdnActFuncHandle gdnActFuncParam;     // parameters for GDN/IGDN act functions

    int16_t featDimIn;                  // dimension of input feature
    int16_t featDimOut;                 // dimension of output feature
    float *featOut;                     // output feature buffer, size: featDimOut * numChannelsOut

    // buffer for runtime
    float *featureInterPolated;         // interpolated feature, only for conv1D transpose, size: featDimIn * stride
    float *featurePadded;               // padded feature
                                        //   conv1D           size: (featDimIn + paddingSize) * numChannelsIn
                                        //   conv1D transpose size: (featDimIn * stride + paddingSize) * numChannelsIn
    float *featureCol;                  // columned feature
                                        //   conv1D           size: (featDimIn / stride) * (kernelSize * numChannelsIn)
                                        //   conv1D transpose size: (featDimIn * stride) * (kernelSize * numChannelsIn)
    float *kernelCol;                   // columned kernel
                                        //   conv1D           size: (kernelSize * numChannelsIn) * numChannelsOut
                                        //   conv1D transpose size: (kernelSize * numChannelsIn) * numChannelsOut

    // runtime buffer for conv1D transpose
    // with stride = 2, odd and even part
    float *kernelColOdd;                 // columned kernel, for conv1D transpose, stride = 2
                                         //   size: ((kernelSize + 1) / 2 * numChannelsIn) * numChannelsOut
    float *kernelColEven;                // columned kernel, for conv1D transpose, stride = 2
                                         //   size: ((kernelSize - 1) / 2 * numChannelsIn) * numChannelsOut
    float *featureColOdd;                // columned feature, for conv1D transpose, stride = 2
                                         //   size: featDimIn * ((kernelSize + 1) / 2 * numChannelsIn))
    float *featureColEven;               // columned feature, for conv1D transpose, stride = 2
                                         //   size: featDimIn * ((kernelSize - 1) / 2 * numChannelsIn))
    float *featOutOdd;                   // temporary output feature buffer, for conv1D transpose, stride = 2
                                         //   size: featDimIn * numChannelsOut, Note: featDimIn = featDimOut / 2
    float *featOutEven;                  // temporary output feature buffer, for conv1D transpose, stride = 2
                                         //   size: featDimIn * numChannelsOut, Note: featDimIn = featDimOut / 2

}CnnStruct, *CnnStructHandle;

// model structure, now for pure cnn layers
// could be used for both encoder and decoder
typedef struct ModelStructure {
    int16_t numLayers;

    CnnStructHandle cnnLayers[MAX_LAYERS];
}ModelStruct, *ModelStructHandle;

// top level structure for neural based QC
typedef struct NeuralCodecStructure {

    int16_t numLinesEncode;                         // number of input feature
    int16_t numLatentEncode;                        // dim of latent params
    int16_t numLatentChannels;                      // number of channels for latent params

    // Encoder handle
    ModelStructHandle encoderHandle;

    // Decoder handle
    ModelStructHandle decoderHandle;

    // Quantizer handle
    QuantizerHandle quantizerHandle;

    // RC handle
    RangeCoderConfigHandle rangeCoderConfig;        // RC config

    // Context scale, only for hyper-prior now
    int16_t numContextScale;
    float *contextScale;
}NeuralCodecStruct, *NeuralCodecHandle;

// neural based QC data structure, for each channel
typedef struct NeuralQcDataStructure {

    uint8_t baseBitstream[MAX_QC_BS_LENGTH];
    int16_t baseNumBytes;

    uint8_t contextBitstream[MAX_QC_BS_LENGTH];
    int16_t contextNumBytes;

    float featureScale;
    int16_t isFeatAmplified;
    int16_t scaleQIdx;

    float nfParam[N_GROUP_SHORT_WIN];
    int16_t nfParamQIdx[N_GROUP_SHORT_WIN];
}NeuralQcData;


// TNS structures
// filter type
typedef enum
{
    TNS_FILTER_SYNTHESIS = 0,
    TNS_FILTER_ANALYSIS = 1
} TnsFilterType;

// tns filter param
typedef struct TnsFilterStructure
{
    short order;                            // tns filter order
    short coefIndex[TNS_MAX_FILTER_ORDER];  // quantized tns filter coeff (parcor)
} TnsFilter;

// tns bitstream param
typedef struct TnsBitstreamStructure
{
    short enable;                                               // tns enable flag
    short order;                                                // tns filter order
    short parcorNbits[TNS_MAX_FILTER_ORDER];                    // nbits for each parcor order
    unsigned short parcorHuffCode[TNS_MAX_FILTER_ORDER];        // code for each parcor order
} TnsBsParam;

// tns detector
typedef struct TnsDetectorStructure
{
    float predictionGain;                   // tns prediction gain
    float avgSqrCoef;                       // average squared parcor coeff
} TnsDetector;

// tns data
typedef struct
{
    TnsDetector tnsDetector[TNS_MAX_FILTER_NUM];        // tns detectors
    TnsFilter filter[TNS_MAX_FILTER_NUM];               // tns filters
    TnsBsParam bsParam[TNS_MAX_FILTER_NUM];             // tns bitstream parameters
} TnsData;

// huffman codes
typedef struct
{
    unsigned char value;
    unsigned short code;
    unsigned char nBits;
} TnsHuffman;

typedef enum _BweRateIndex {
    BWE_BITRATE_FB_MONO_32K = 0,
    BWE_BITRATE_FB_MONO_48K,
    BWE_BITRATE_FB_MONO_64K,
    BWE_BITRATE_FB_MONO_96K,
    BWE_BITRATE_FB_STEREO_48K,
    BWE_BITRATE_FB_STEREO_64K,
    BWE_BITRATE_FB_STEREO_96K,
    BWE_BITRATE_FB_STEREO_128K,
    BWE_BITRATE_FB_MC_CPE_48K,
    BWE_BITRATE_FB_MC_CPE_64K,
    BWE_BITRATE_FB_MC_CPE_96K,
    BWE_BITRATE_FB_MC_CPE_128K,
    BWE_BITRATE_FB_HOA_ELOW,
    BWE_BITRATE_FB_HOA_LOW,
    BWE_BITRATE_FB_HOA_MIDDLE,
    BWE_BITRATE_FB_HOA_HIGH,
    BWE_BITRATE_FB_UNKNOWN
}BweRateIndex;

typedef enum _BweWhiteningLevel {
    BWE_WHITENING_OFF = 0,
    BWE_WHITENING_MID = 1,
    BWE_WHITENING_HIGH = 2
}BweWhiteningLevel;

// bwe config structure
typedef struct BweConfigStructure {

    int16_t numTiles;                           // number of tiles for bwe
    int16_t numSfb;                             // number of sfbs for bwe

    int16_t bweStartLine;                       // start bin index of bwe
    int16_t bweStopLine;                        // stop bin index of bwe

    int16_t targetTiles[MAX_NUM_TILE + 1];      // target tile division
    int16_t srcTiles[MAX_NUM_TILE];             // source tile start line
    int16_t sfbTable[MAX_NUM_SFB_BWE + 1];      // SFB division
    int16_t sfbTileWrap[MAX_NUM_TILE + 1];      // sfb-tile ralation

}BweConfigData, *BweConfigHandle;

/* BWE decoder side data structure */
typedef struct BweDecDataStructure {

    float sfbEnvelope[MAX_NUM_SFB_BWE];         // quantized sfb envelope
    int16_t sfbEnvQIdx[MAX_NUM_SFB_BWE];        // sfb envelope quantization index
    int16_t whiteningLevel[MAX_NUM_TILE];       // tile whitening level

    float bweSpectrum[BLOCK_LEN_LONG];          // buffer of whitened spectrum

}BweDecData, *BweDecDataHandle;

/* MCR config structure */
typedef struct mcr_config_structure {

    // 2 for long/short window
    int16_t numSfb[MCR_NUM_CONFIG];                      // num sfbs
    int16_t vqVecNum[MCR_NUM_CONFIG];                    // num of subvectors for vq
    int16_t vqVecDim[MCR_NUM_CONFIG];                    // dim of subvectors for vq
    int16_t vqNumBits[MCR_NUM_CONFIG];                   // number of vq bits for each subvector
    int16_t vqCbSize[MCR_NUM_CONFIG];                    // vq codebook size
    const float *vqCodebook[MCR_NUM_CONFIG];             // vq codebook pointers

}MCR_CONFIG, *MCR_CONFIG_HANDLE;

/* MCR data structure */
typedef struct mcr_data_structure {

    // 2 for even/odd spectrum
    // 0 for even, 1 for odd
    float theta[MCR_NUM_SUBSPEC][MCR_NUM_SFB_FB];             // mcr angle
    int16_t vqIdx[MCR_NUM_SUBSPEC][MCR_NUM_SUBVEC];           // vq idx for mcr angle

}MCR_DATA, *MCR_DATA_HANDLE;

// Matrix struct for GEMM
typedef struct {
    const float *data;
    int row;
    int col;
} MatrixStruct;

typedef struct {
    float *data;
    int row;
    int col;
} MatrixStructUnconst, *MatrixHandleUnconst;

typedef struct _modul_structure{
	unsigned char data[79930];
	unsigned int nIndex;
}modul_structure;

#endif // AVS3_STAT_COM_H
