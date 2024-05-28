#ifndef AVS3_CNST_COM_H
#define AVS3_CNST_COM_H

#include "avs3_options.h"

#define AVS3_MAX(a,b) (((a) > (b)) ? (a) : (b))
#define AVS3_MIN(a,b) (((a) < (b)) ? (a) : (b))

#ifndef __cplusplus
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define AVS3_FALSE    0
#define AVS3_TRUE     1
#define AVS3_DATA_NOT_ENOUGH	2

#define AVS3_PI              3.14159265358979323846264338327950288f
#define AVS3_EPSILON         0.000000000000001f

#define AVS3_SAMPLING_48KHZ  48000

#define STEREO_CHANNELS      2
#define MAX_CHANNELS         16

/* Config table sizes */
#define AVS3_SIZE_BITRATE_TABLE         16
#define AVS3_SIZE_MC_CONFIG_TABLE       10
#define AVS3_SIZE_FS_TABLE              9

/* Header bits */
#define SYNC_WORD_COMPAT                0xFFF
#define NBITS_SYNC_WORD                 12
#define NBITS_AUDIO_CODEC_ID            4
#define NBITS_ANC_DATA_INDEX            1
#define NBITS_NN_TYPE                   3
#define NBITS_CODING_PROFILE            3
#define NBITS_SAMPLING_RATE_INDEX       4
#define NBITS_CHANNEL_NUMBER_INDEX      7
#define NBITS_SOUNDBED_TYPE             2
#define NBITS_NUM_OBJS                  7
#define NBITS_HOA_ORDER                 4
#define NBITS_RESOLUTION                2
#define NBITS_BITRATE_INDEX             4

/* Total header bits for each mode */
#define NBITS_FRAME_HEADER_MONO         56      // 12+4+1+3+3+4+8+7+2+4+8, 56, pad to 56
#define NBITS_FRAME_HEADER_STEREO       56      // 12+4+1+3+3+4+8+7+2+4+8, 56, pad to 56
#define NBITS_FRAME_HEADER_MC           56      // 12+4+1+3+3+4+8+7+2+4+8, 56, pad to 56
#define NBITS_FRAME_HEADER_HOA          56      // 12+4+1+3+3+4+8+4+2+4+8, 53, pad to 56
#define NBITS_FRAME_HEADER_MIX_SBT0     64      // 12+4+1+3+3+4+8+2+7+4+2+8, 58, pad to 64, mix, soundbed type 0
#define NBITS_FRAME_HEADER_MIX_SBT1     72      // 12+4+1+3+3+4+8+2+7+4+7+4+2+8, 69, pad to 72, mix, soundbed type 1
#define MAX_NBYTES_FRAME_HEADER         9       // maximum num bytes for frame header, in mix mode

/* Bandwidth */
#define SWB 0
#define FB  1

/* AVS3 Supported Format */
#define AVS3_MONO_FORMAT   0
#define AVS3_STEREO_FORMAT 1
#define AVS3_MC_FORMAT     2
#define AVS3_HOA_FORMAT    3
#define AVS3_MIX_FORMAT    4

/* Inner HOA extend format */
#define AVS3_INNER_FOA_FORMAT   5
#define AVS3_INNER_HOA2_FORMAT  6
#define AVS3_INNER_HOA3_FORMAT  7

/* Codec core format */
#define AVS3_MDCT_CORE     0

/* AVS3 stereo mode */
#define AVS3_MDCT_STEREO   0

/* Frame size */
#define N_SAMPLES_LOOKAHEAD 512
#define FRAME_LEN           1024
#define MAX_FRAME_LEN       (FRAME_LEN + N_SAMPLES_LOOKAHEAD)
#define BLOCK_LEN_LONG      1024
#define BLOCK_LEN_HALF_LONG 512
#define BLOCK_LEN_SHORT     128
#define MAX_OVL_SIZE        1024
#define BLOCK_PADDING_SIZE  448
#define N_BLOCK_SHORT       (BLOCK_LEN_LONG/BLOCK_LEN_SHORT)

/* Grouping */
#define N_GROUP_SHORT_WIN   2           // number of groups for short window frame

/* Window types */
#define ONLY_LONG_WINDOW           0
#define ONLY_SHORT_WINDOW          1
#define LONG_SHORT_TRANS_WINDOW    2
#define SHORT_LONG_TRANS_WINDOW    3
#define NBITS_TRANSFORM_TYPE       2            // nbits for transform type

/* constants for transient detection */
#define NUM_BLOCKS 8                // number of blocks in detection
#define HP_ORDER 2                  // highpass filter order
#define NUM_TRANS_HIST 2            // length of trans detection history
#define MIN_ENERGY 103.37f          // minimum block energy
#define ATTENUATION_COEFF 0.8125f   // attenuation factor of block energy
#define TH_ENERGY_COEFF 8.0f        // energy burst threshold

/* FFT */
#define NORM_MDCT_FACTOR          0.5f
#define FACTOR_TWIDDLE_SHORT      0.125f
#define N_FFT_TABLE               2
#define FFT_TABLE_SIZE32          32
#define FFT_TABLE_SIZE64          64
#define FFT_TABLE_SIZE128         128
#define FFT_TABLE_SIZE256         256
#define FFT_TABLE_SIZE512         512
#define MAX_FFT_TABLE_SIZE        FFT_TABLE_SIZE512

/* HOA */
#define HOA_LEN_FRAME48k          FRAME_LEN
#define MAX_HOA_CHANNELS          MAX_CHANNELS
#define HOA_OVERLAP_SIZE          512
#define HOA_LEN_TRANSFORM         FRAME_LEN + HOA_OVERLAP_SIZE
#define HOA_MAX_FORE_CHANS        4
#define MAX_HOA_BASIS             HOA_MAX_FORE_CHANS
#define L_HOA_BASIS_ROWS          MAX_HOA_CHANNELS
#define HOA_RCOND                 0.0625f
#define L_HOA_BASIS_COLS          1343
#define L_FIRST_ORDER_HOA_BASIS   108
#define L_SECOND_ORDER_MP_BASIS   127
#define L_SIN_TABLE_256           256
#define L_SIN_TABLE_512           (L_SIN_TABLE_256*2)
#define L_SIN_TABLE_768           (L_SIN_TABLE_256*3)
#define L_SIN_TABLE_1024          (L_SIN_TABLE_256*4)
#define HOA_BASIS_BITS            12
#define N_BLOCK_HOA               2
#define HOA_DELAY_BASIS           2

/* HOA DMX */
#define HOA_MAX_SFB               45
#define MAX_HOA_DMX_CHANNELS      MAX_HOA_CHANNELS
#define MAX_HOA_DMX_GROUPS        3
#define HOA_ILD_BITS              5
#define HOA_RATIO_BITS            4
#define HOA_BITS_RATIO_RANGE      (1<<HOA_RATIO_BITS) 

#define DMX_MONO                  0
#define DMX_FULL_MS               1
#define DMX_SFB_MS                2
#define N_SFB_HOA_LBR             22
#define CUTOFF_BANDS              3

#define FOA_BITRATE_48K            48000
#define FOA_BITRATE_96K            96000
#define HOA_BITRATE_192K          192000
#define HOA_BITRATE_240K          240000
#define HOA_BITRATE_256K          256000
#define HOA_BITRATE_320K          320000
#define HOA_BITRATE_384K          384000
#define HOA_BITRATE_480K          480000
#define HOA_BITRATE_512K          512000
#define HOA_BITRATE_608K          608000
#define HOA_BITRATE_640K          640000
#define HOA_BITRATE_768K          768000
#define HOA_BITRATE_896K          896000
#define HOA_SIZE_BITRATE_TABLE    13

/* Stereo mode */
#define TH_CROSS_CORR            0.3f                           // threshold of normalized cross correlation

#define NBITS_MS_FLAG            1                              // nbits for ms flag
#define NBITS_ENERGY_BALENCE     4                              // nbits for ILD
#define ENERGY_BALENCE_RANGE     (1<<NBITS_ENERGY_BALENCE)      // range of ILD

#define NBITS_SPLIT_STEREO       3                              // nbits for stereo bit split ratio
#define BITS_SPLIT_RANGE         (1<<NBITS_SPLIT_STEREO)        // range of stereo bit split ratio
#define EQUAL_BITS_RATIO         (BITS_SPLIT_RANGE>>1)          // ratio of equal bit split

#define LR_ENGERY_RATIO_H         3.0f
#define LR_ENGERY_RATIO_L         1.0f/LR_ENGERY_RATIO_H


// MC mode
#define LFE_CHANNEL_INDEX                   3                   // LFE channel idx
#define WHOLE_COUPLE_THREHOLD               0.5f                // coupling threhold

#define MC_EB_BITS                          5                       // nbits for energy balence
#define MC_ILD_CBLEN                        30                      // mc ild codebook length
#define PAIR_NUM_DATA_BITS                  4                       // nbits for pair number
#define NBITS_MC_RATIO                      6                       // nbits for MC bit allocation ratio
#define BITRATE_MC_RATIO_SCOPE              (1 << NBITS_MC_RATIO)   // range of MC bit allocation ratio
#define LFE_STATUS                          1                       // 1: on  0: off

#define SILENCE_BYTES                       8                       // bytes allocated for silence frame
#define NBITS_SILFLAG                       1                       // nbits for silence flag of each channel
#define NBITS_HASSILFLAG                    1                       // nbits for has silence flag for current frame

#define BIT_FRAME_MAX                       (256000 * FRAME_LEN/ AVS3_SAMPLING_48KHZ)   // max number of bits for each frame/channel
#define LFE_RESERVED_LINES                  32                  // reserved lines for LFE channel, 32 for 750Hz in long window


// Neural QC
#define MAX_LAYERS 10           // maximum layers of encoder/decoder
#define MAX_RATE_ITERS 6        // maximum number of iterations in rate loop
#define MAX_QC_BS_LENGTH 1024   // maximum QC bitstream length in Bytes

#define NBITS_IS_FEAT_AMPLIFIED     1       // nbits for isFeatAmplified
#define NBITS_FEATURE_SCALE         7       // nbits for feature scale
#define NBITS_FEATURE_SCALE_LC      8       // nbits for feature scale, use all 8 bits
#define NBITS_NF_PARAM              3       // nbits for NF
#define NBITS_CONTEXT_NUM_BYTES     8       // nbits for context number bytes


/* Spectrum Shaping */
#define LPC_ORDER           16                      /* The order of the LPC */
#define LSP_ROOT_NUM        LPC_ORDER / 2           /* Number of roots in LSP calculation */
#define PREEMPH_FAC_FB      0.9f                    /* Preemphasis factor for full-band */
#define NUM_ITER            4                       /* Number of iteration in LSP calculation */
#define GRID_100_POINTS     100                     /* LP analysis - number of points to evaluate Chebyshev polynomials */
#define GAMMA_LPC           0.939999998f            /* LP weighting factor */
#define N_SFB_FB_LONG       49                      // number of sfbs for FB long window case

#define MAX_CANDIDATE_NUM       4                   // max number of candidates in 1st round VQ

#define LSF_CB_NUM_HBR          7                   // number of LSF codebooks
#define LSF_STAGE1_LEN1_HBR     9                   // 1st subvector dim in stage 1
#define LSF_STAGE1_LEN2_HBR     7                   // 2nd subvector dim in stage 1
#define LSF_STAGE2_LEN1_HBR     3                   // 1st subvector dim in stage 2
#define LSF_STAGE2_LEN2_HBR     3                   // 2nd subvector dim in stage 2
#define LSF_STAGE2_LEN3_HBR     3                   // 3rd subvector dim in stage 2
#define LSF_STAGE2_LEN4_HBR     3                   // 4th subvector dim in stage 2
#define LSF_STAGE2_LEN5_HBR     4                   // 5th subvector dim in stage 2

#define LSF_STAGE1_CB1_SIZE_HBR     256             // CB size of stage 1 subvector 1
#define LSF_STAGE1_CB2_SIZE_HBR     256             // CB size of stage 1 subvector 2
#define LSF_STAGE2_CB1_SIZE_HBR     128             // CB size of stage 2 subvector 1
#define LSF_STAGE2_CB2_SIZE_HBR     128             // CB size of stage 2 subvector 2
#define LSF_STAGE2_CB3_SIZE_HBR     64              // CB size of stage 2 subvector 3
#define LSF_STAGE2_CB4_SIZE_HBR     32              // CB size of stage 2 subvector 4
#define LSF_STAGE2_CB5_SIZE_HBR     32              // CB size of stage 2 subvector 5

#define LSF_STAGE1_CB1_NBITS_HBR    8               // VQ nbits of stage 1 subvector 1
#define LSF_STAGE1_CB2_NBITS_HBR    8               // VQ nbits of stage 1 subvector 2
#define LSF_STAGE2_CB1_NBITS_HBR    7               // VQ nbits of stage 2 subvector 1
#define LSF_STAGE2_CB2_NBITS_HBR    7               // VQ nbits of stage 2 subvector 2
#define LSF_STAGE2_CB3_NBITS_HBR    6               // VQ nbits of stage 2 subvector 3
#define LSF_STAGE2_CB4_NBITS_HBR    5               // VQ nbits of stage 2 subvector 4
#define LSF_STAGE2_CB5_NBITS_HBR    5               // VQ nbits of stage 2 subvector 5

#define LSF_Q_LBR_THRESH        32000                   // bitrate threshold to use different CBs
#define LSF_CB_NUM_LBR          5                       // number of LSF codebooks with 36 bits
#define LSF_STAGE1_LEN1_LBR     9                       // 1st subvector dim in stage 1 with 36 bits
#define LSF_STAGE1_LEN2_LBR     7                       // 2nd subvector dim in stage 1 with 36 bits
#define LSF_STAGE2_LEN1_LBR     5                       // 1st subvector dim in stage 2 with 36 bits
#define LSF_STAGE2_LEN2_LBR     4                       // 2nd subvector dim in stage 2 with 36 bits
#define LSF_STAGE2_LEN3_LBR     7                       // 3rd subvector dim in stage 2 with 36 bits

#define LSF_STAGE1_CB1_SIZE_LBR     256                 // CB size of stage 1 subvector 1 with 36 bits
#define LSF_STAGE1_CB2_SIZE_LBR     256                 // CB size of stage 1 subvector 2 with 36 bits
#define LSF_STAGE2_CB1_SIZE_LBR     128                 // CB size of stage 2 subvector 1 with 36 bits
#define LSF_STAGE2_CB2_SIZE_LBR     128                 // CB size of stage 2 subvector 2 with 36 bits
#define LSF_STAGE2_CB3_SIZE_LBR     64                  // CB size of stage 2 subvector 3 with 36 bits

#define LSF_STAGE1_CB1_NBITS_LBR    8                   // VQ nbits of stage 1 subvector 1 with 36 bits
#define LSF_STAGE1_CB2_NBITS_LBR    8                   // VQ nbits of stage 1 subvector 2 with 36 bits
#define LSF_STAGE2_CB1_NBITS_LBR    7                   // VQ nbits of stage 2 subvector 1 with 36 bits
#define LSF_STAGE2_CB2_NBITS_LBR    7                   // VQ nbits of stage 2 subvector 2 with 36 bits
#define LSF_STAGE2_CB3_NBITS_LBR    6                   // VQ nbits of stage 2 subvector 3 with 36 bits

#define LSF_MIN_GAP             50.0f               // Minimum LSF gap allowed

// TNS
#define TNS_MAX_FILTER_NUM              2         // TNS: max number of filters
#define TNS_MAX_FILTER_ORDER            8         // TNS: max order of tns filter
#define TNS_DIVISION_NUM                3         // TNS: number of spectrum divisions in autocorr calculation

#define HLM_MIN_NRG                     32768.0f                    // TNS: minimum energy in each spectrum division
#define TNS_COEFF_RES                   4                           // TNS: resolution of parcor quantization, 2^4 steps
#define INDEX_SHIFT                     (1 << (TNS_COEFF_RES-1))    // TNS: used to shift quantization index to zero
#define N_TNS_COEFF_CODES               16                          // TNS: number of huffman codes for parcor

#define MIN_AVG_SQR_COEFF_THREHOLD      0.06f     // TNS: in tns detect, threshold for averaged square parcor coeff.
#define MIN_PREDICTION_GAIN             1.35f     // TNS: in tns detect, threshold for prediction gain

#define TNS_NBITS_ENABLE                1         // TNS: nbits for filter enable
#define TNS_NBITS_ORDER                 3         // TNS: nbits for filter order

// BWE
#define MAX_NUM_TILE        4       // BWE: max number of tiles in high band
#define MAX_NUM_SFB_BWE     8       // BWE: max number of sfbs in high band
#define LEN_HIST_WHITENING  3       // BWE: history length in whitening decision
#define LEN_WHITEN_AVERAGE  7       // BWE: half whitening length of MIDDLE level

#define NBITS_BWE_ENV               7       // BWE: nbits for bwe envelope
#define NBITS_BWE_WHITEN_ONOFF      1       // BWE: nbits for bwe whitening ON/OFF flag
#define NBITS_BWE_WHITEN_MIDHIGH    1       // BWE: nbits for bwe whitening MID/HIGH flag

/* Bitstream */
#define AVS3_BS_BYTE_SIZE        8
#define MAX_BS_BYTES             12300                    // max condition: for 16obj, 192kbps each, 32kHz sampling rate, 12288 bytes
#define MAX_NUM_BS_PARTS         (MAX_CHANNELS + 4)       // max number of bitstream parts in encoder, 4 stands for STEREO/MC/HOA/META
#define BS_INDEX_STEREO          (MAX_CHANNELS + 1 - 1)   // bitstream part index for stereo side info
#define BS_INDEX_MC              (MAX_CHANNELS + 2 - 1)   // bitstream part index for MC side info
#define BS_INDEX_HOA             (MAX_CHANNELS + 3 - 1)   // bitstream part index for HOA side info
#define BS_INDEX_META            (MAX_CHANNELS + 4 - 1)   // bitstream part index for Meta data side info
#define MAX_NUM_INDICES          1000                     // max number of indices in each bitstream handle

/* MCR stereo */
#define TH_BR_MCR_STEREO        32000           // bitrate threshold for MCR stereo mode
#define MCR_NUM_SUBSPEC         2               // number of sub-spec for MCR, odd and even spectrum now
#define MCR_NUM_CONFIG          2               // number of MCR configs, short win and long win (includes transition)
#define MCR_NUM_SFB_FB          18              // number of sfbs
#define MCR_DIM_SUBVEC          3               // number of sfbs in each subvector for theta vq
#define MCR_NUM_SUBVEC          6               // number of subvectors for theta vq
#define MCR_VQ_NBITS_LONG       9               // num bits for mcr vq in long window frame (also transition frame)
#define MCR_VQ_NBITS_SHORT      8               // num bits for mcr vq in short window frame
#define MCR_VQ_CBSIZE_LONG      512             // size of VQ codebook for mcr, 9bit
#define MCR_VQ_CBSIZE_SHORT     256             // size of VQ codebook for mcr, 8bit

#ifdef ANDROID
#include <android/log.h>
#define TAG "av3a_decoder" // 这个是自定义的LOG的标识   
#define LOGD(format, ...) //__android_log_print(ANDROID_LOG_DEBUG,TAG ,format,##__VA_ARGS__) // 定义LOGD类型  
#else
#define LOGD(format, ...) printf(format, ##__VA_ARGS__)
#endif

#endif