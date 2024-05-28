#ifndef AVS3_OPTIONS_H
#define AVS3_OPTIONS_H

// #define DEBUG_AVS3_HOA            /* shuai: Debug AVS3 HOA init version */
// #define BUG_FIXED_AVS3_HOA        /* shuai: Fixed HOA angle pair compute error. */
// #define AVS3_WAVE_READER          /* shuai: Fixed wave file errors. */
// #define LOW_DELAY_AVS3_HOA        /* shuai: Supported Low delay HOA encoding routine. */
// #define TRANSIENT_DETECT          // for transient detection integration
// #define DEBUG_AVS3_MDCT_TRANSFORM /* shuai: Local synthesis time domain signal */
// #define DEBUG_AVS3_MONO           /* shuai: Support Mono format in AVS3 */
// #define SIMULATING_HOA_DECODER    /* shuai: Simulating the integrated HOA decoding routine in encoder. */
// #define DEBUG_AVS3_BITSTREAM      /* shuai: Bitstream I/O support for AVS3 encoder to decoder. */
// #define AVS3_HOA_DMX              /* HOA DMX */
//#define DEBUG_NEURAL_QC_LOCAL_SYNTH

#define NEURAL_QC
#define FD_SHAPING                  /* djc/xby: LPC based spectrum shaping */
//#define POST_SHAPING                /* ljw/xby: post FD shaping in low bitrate */
#define TD_SHAPING                  /* xby: TNS integration */
#define TNS_SHORT_WIN               /* xby: for short window frame, deinterleave before TNS filtering, interleave after TNS */
#define BWE_DEVELOPE                /* xby: BWE integration */

#define MONO_INTEGRATE              /* xby: Mono mode integration */

#define MIX_DEVELOPE                /* xby/shuai: Mix mode developement */
#define MIX_EXT
//#define DEBUG_MIX_DEVELOPE
#define METADATA_DEVELOPE           /* xby: metadata enc/dec */
#define METADATA_EXT                /* mxb/xby: ADM+ext metadata */
//#define DEBUG_META

#define MC_ENABLE                   /* wangzhi */
#define MC_LFE_PROC                 /* wangzhi, clean LFE channel HF spectrum in both encoder/decoder */
#define MC_TUNING                   /* wangzhi, adjust bwe band config and bwe bitrate index */
#define MC_ALLOC_BITS_LIMIT         /* wangzhi, make sure the actual allocated num bits is below limit (256kbps/ch) */

#define CRC_CHECK                   /* xby: CRC calculation, bs writing and check */

#define AVS3_HOA_TUNING             /* shuai: AVS3 HOA tuning */
#define AVS3_HOA_VL_SELECTION 
#define AVS3_HOA_BWE                /* shuai: AVS3 HOA BWE tuning */

#define BWE_TUNING                  /* xby: bwe config tuning */

#define AVS3_EXTENSION_WAV_READ     /* shuai: bug fixed for wav header with extension RIFF sub-chunk. */

#define BS_HEADER_COMPAT            /* xby: AVS2P3 compatible bs header definition */

#define AVS3_HOA_BUG_FIXED          /* shuai: AVS3 HOA bug fixed */

#define BITRATE_EXT                 /* xby: extention of bitrate table */

#define MCR_INTEGRATE               /* xby: MCR for low bitrate stereo coding */

#define AVS3_HOA_FULL_SUPPORT       /* shuai: support FOA to HOA3 */
#define HOA_BUGFIX
#define HOA_BUGFIX_UPMIX

#define BRTABLE_ALIGN               /* xby: Align bitrate table with WD, for mc 5.1 and mono bwe config */

#define SUPPORT_MORE_FS             /* xby: support sample rate other than 48kHz */
#define SUPPORT_24BIT_INPUT         /* shuai: support 24bit wav file input */

#define BUGFIX_FOA_RELEASE          /* shuai: bugfix for FOA release crash */

#define BITRATE_EST                 /* ljw/xby: bitrate estimate for RC */

#define SUPPORT_NNTYPE_LC           /* xby/ljw: add nn type to bitstream, integrate LC profile */

#define SUPPORT_HIGH_BR_MIX         /* xby: support higher bitrate for mix mode, exceeding bitsPerFrame short range */

#define BUGFIX_METADATA             /* wangzhi, metadata quantization at encoder side */
#define BUGFIX_METADATA_MXB         /* mxb: metadata quantization and coding bugfix */
#define METADATA_UPDATE             /* wangzhi, update dynamic meta */

#define BUGFIX_LOCAL_SYNTH          /* xby: fix NF param quantization bug, found in local synth, NBE */
#define BUGFIX_MC_16CH              /* wangzhi: fix MC 16ch crash bug in release mode */

#define IMPR_MIX_BIT_ALLOC          /* wangzhi: improvements on mix/mc mode bit allocation */
#define SWITCH_MIX_BITALLOC_MODE    /* wangzhi: for production and standard bit allocation mode, on for production */
#define ONLY_PAIR_ILD               /* wangzhi: use pair ild only in mcac */

#define MC_ILD_CBQUANT              /* Quantize MC ild using codebook */
#define PAIR_INFOR_ORDER            /* wangzhi: MC format, reorder pair info in bs */

#define HOA_ILD_CBQUANT             /* Quantize HOA ild using codebook */

#endif
  