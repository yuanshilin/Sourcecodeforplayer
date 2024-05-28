#ifndef AVS3_STAT_DEC_H
#define AVS3_STAT_DEC_H
#include <stdio.h>
#include "avs3_options.h"
#include "avs3_cnst_com.h"
#include "avs3_stat_com.h"

#ifdef METADATA_EXT
#include "avs3_stat_meta.h"
#endif

/* bitstream buffer */
typedef struct avs3_dec_bitstream_structure 
{
    uint8_t bitstream[MAX_BS_BYTES];
    uint32_t nextBitPos;

}AVS3_BSTEREAM_DEC_DATA, *AVS3_BSTEREAM_DATA_DEC_HANDLE;

#ifdef METADATA_EXT
typedef struct Avs3MetaDataDecStructure {
    Avs3MetaData  avs3MetaData;
}Avs3DecMetadata, *Avs3DecMetadataHandle;
#endif

typedef struct avs3_hoa_dec_structure 
{
    float decHoaDelayBuffer[MAX_HOA_CHANNELS][HOA_LEN_FRAME48k + HOA_LEN_FRAME48k];
    float decSpecturm[MAX_HOA_CHANNELS][HOA_LEN_FRAME48k];
    float decSynthBuffer[MAX_HOA_CHANNELS][HOA_OVERLAP_SIZE];
    float* decSignalInput[MAX_HOA_CHANNELS];
    
    short basisIdx[MAX_HOA_BASIS];
    short delayBasisIdx[HOA_DELAY_BASIS][MAX_HOA_BASIS];

    short bitsRatio[MAX_HOA_DMX_GROUPS][MAX_HOA_DMX_CHANNELS];
    short groupILD[MAX_HOA_DMX_CHANNELS];
#ifndef HOA_ILD_CBQUANT
    short flagNrg[MAX_HOA_DMX_CHANNELS];
#endif
    short chIdx[MAX_HOA_DMX_GROUPS][MAX_HOA_DMX_CHANNELS / 2];
    short pairIdx[MAX_HOA_DMX_GROUPS];
    short lastPairIdx[MAX_HOA_DMX_GROUPS];
    short groupBitsRatio[MAX_HOA_DMX_GROUPS];

#ifdef AVS3_HOA_BUG_FIXED
    short dmxMode[MAX_HOA_DMX_GROUPS][MAX_HOA_DMX_CHANNELS / 2];
    short sfbMask[MAX_HOA_DMX_GROUPS][MAX_HOA_DMX_CHANNELS / 2][N_SFB_HOA_LBR];
#endif

#ifdef AVS3_HOA_FULL_SUPPORT
    short numVL;
    short sceneType;
#else
    short numVote;
#endif

    AVS3_HOA_CONFIG_DATA_HANDLE hHoaConfig;

}AVS3_HOA_DEC_DATA, *AVS3_HOA_DEC_DATA_HANDLE;

typedef struct avs3_dec_core_structure
{
    /* frame length */
    short frameLength;

    float synthBuffer[MAX_OVL_SIZE];

    /* original spectrum */
    float origSpectrum[FRAME_LEN];

    /* transform type */
    short transformType;

    /* Mdct grouping info */
    short numGroups;
    short groupIndicator[NUM_BLOCKS];

#ifdef FD_SHAPING
    /* FD spectrum shaping */
    short lsfVqIndex[LSF_CB_NUM_HBR];           // lsf VQ indices
    short lsfLbrFlag;                           // low bitrate index for lsf VQ
#endif

#ifdef TD_SHAPING
    /* TNS handle */
    TnsData tnsData;
#endif

#ifdef BWE_DEVELOPE
    /* BWE */
    int16_t bwePresent;                     // bwe present flag
    BweConfigData bweConfig;                // bwe config info
    BweDecData bweDecData;                  // bwe encoding data
#endif

    NeuralQcData neuralQcData;

    AVS3_CORE_CONFIG_DATA_HANDLE hCoreConfig;

}AVS3_DEC_CORE_DATA, *AVS3_DEC_CORE_HANDLE;

/* AVS3 MDCT Stereo structure */
typedef struct avs3_stereo_dec_data_structure
{
    float* mdctSpectrum[STEREO_CHANNELS];
    short isMS;
    short bitsRatio;
    short ILD;

#ifdef MCR_INTEGRATE
    // for mcr mode in low bitrate
    int16_t useMcr;
    MCR_CONFIG mcrConfig;
    MCR_DATA mcrData;
#endif

}AVS3_STEREO_DEC_DATA, *AVS3_STEREO_DEC_HANDLE;

#ifdef MONO_INTEGRATE
/* AVS3 MONO structure */
typedef struct avs3_mono_dec_data_structure
{
    float* mdctSpectrum;

}AVS3_MONO_DEC_DATA, *AVS3_MONO_DEC_HANDLE;
#endif

typedef struct avs3_main_decoder_structure
{
    short initFrame;

    long  outputFs;
    short bitDepth;                                 // bit depth or resolution of audio signal, 16/24
    long  totalBitrate;
    long  lastTotalBrate;
#ifdef BS_HEADER_COMPAT
    ChannelNumConfig channelNumConfig;              // channel number config, for bitrate table selection
#endif
    short numChansOutput;
#ifdef MIX_DEVELOPE
    short numObjsOutput;                            // number of input object channels, only for mix mode
#ifdef MIX_EXT
    long bitratePerObj;                             // bitrate for each obj
    long bitrateBedMc;                              // bitrate for mc sound bed
    short soundBedType;                             // type of sound bed, 0 for none (only objs), 1 for mc or hoa
    short isMixedContent;                           // flag for mixed content, i.e. with objects
    short hasLfe;                                   // LFE flag, for mixed content: pure obj or stereo soundbed, no LFE
#endif
#endif
    short avs3CodecFormat;
    short avs3CodecCore;
    short bwidth;
    short frameLength;
#ifndef SUPPORT_HIGH_BR_MIX
    short bitsPerFrame;
#else
    int32_t bitsPerFrame;
#endif

#ifdef SUPPORT_NNTYPE_LC
    NnTypeConfig nnTypeConfig;                      // neural network model type config
#endif

    /* Neural QC model type and handles */
    TypeModel modelType;
    NeuralCodecHandle baseCodecSt;
    NeuralCodecHandle contextCodecSt;

    /* Bitstream buffer */
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream;

    /* HOA */
    AVS3_HOA_DEC_DATA_HANDLE hDecHoa;

#ifdef MC_ENABLE
    /* Mc */
    AVS3_MC_DEC_HANDLE hMcDec;
#endif

    /* Stereo */
    AVS3_STEREO_DEC_HANDLE hDecStereo;

#ifdef MONO_INTEGRATE
    /* Mono */
    AVS3_MONO_DEC_HANDLE hDecMono;
#endif

    /* Decoder core */
    AVS3_DEC_CORE_HANDLE hDecCore[MAX_CHANNELS];

#ifdef METADATA_EXT
    /* Metadata */
    Avs3DecMetadataHandle hMetadataDec;
#endif

	int bInited;
	FILE* fModel;
	int32_t bitsHeader;

}AVS3Decoder, *AVS3DecoderHandle;

#endif